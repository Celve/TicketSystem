#include "storage/index/b_plus_tree_nts.h"

#include <string>

#include "common/macros.h"
#include "storage/page/header_page.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREENTS_TYPE::BPlusTreeNTS(std::string name, BufferPoolManager *buffer_pool_manager,
                                const KeyComparator &comparator, int leaf_max_size, int internal_max_size)  // NOLINT
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  header_page->SearchRecord(index_name_, &root_page_id_);
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/**
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::IsEmpty() const { return root_page_id_ == INVALID_PAGE_ID; }

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/**
 * @brief
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::GetValue(const KeyType &key, vector<ValueType> *result, Transaction *transaction) {
  Page *leaf_page = FindLeafPage(key, false);
  if (leaf_page == nullptr) {
    return false;
  }
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page);
  ValueType value;
  bool flag = false;
  if (leaf_node->Lookup(key, &value, comparator_)) {
    result->push_back(value);
    flag = true;
  }
  buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), false);
  return flag;
}

/**
 * @brief
 * Find the key-value pairs which greater then the given key using the given rule.
 * @param key
 * @param result
 * @param new_comparator
 * @param transaction
 * @return INDEX_TEMPLATE_ARGUMENTS
 * NOTE: need to be modified
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::GetValue(const KeyType &key, vector<ValueType> *result, const KeyComparator &new_comparator,
                                 Transaction *transaction) {
  Page *leaf_page = FindLeafPage(key, false);
  if (leaf_page == nullptr) {
    return false;
  }
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page);
  ValueType value;

  int index = leaf_node->KeyIndex(key, comparator_);

  auto clear_up = [&]() { buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), false); };

  /* iterate through it to find out the value with same first key */
  while (true) {
    if (index != -1) {
      if (new_comparator(key, leaf_node->KeyAt(index))) {
        clear_up();
        return true;
      }
      result->push_back(leaf_node->GetItem(index++).second);
    }
    if (index == -1 || index == leaf_node->GetSize()) {
      index = 0;
      page_id_t next_page_id = leaf_node->GetNextPageId();

      /* unlatch and unpin */
      clear_up();

      if (next_page_id == INVALID_PAGE_ID) {
        break;
      }

      leaf_page = buffer_pool_manager_->FetchPage(next_page_id);
      leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
    }
  }
  return true;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * insert constant key & value pair into b+ tree
 * @return true for insertion; false for modification
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) {
  if (IsEmpty()) {
    StartNewTree(key, value);
    return true;
  }
  return InsertIntoLeaf(key, value, transaction);
}

/**
 * @brief
 * create a new page, which might be different types
 * @param parent_id
 * @return INDEX_TEMPLATE_ARGUMENTS*
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREENTS_TYPE::NewNode(page_id_t parent_id, IndexPageType page_type) {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(&page_id);

  /* all frames in buffer pool are pinned, however, this is impossible to happen */
  if (page == nullptr) {
    throw std::runtime_error("Cannot fetch a new page.");
  }

  N *node = reinterpret_cast<N *>(page->GetData());
  if (page_type == IndexPageType::INTERNAL_PAGE) {
    node->Init(page_id, parent_id, internal_max_size_);
  } else {
    node->Init(page_id, parent_id, leaf_max_size_);
  }
  return node;
}

/**
 * @brief
 * Insert constant key & value pair into an empty tree
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREENTS_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  LeafPage *node = NewNode<LeafPage>(INVALID_PAGE_ID, IndexPageType::LEAF_PAGE);

  /* insert */
  node->Insert(key, value, comparator_);

  page_id_t page_id = node->GetPageId();
  buffer_pool_manager_->UnpinPage(page_id, true);
  root_page_id_ = page_id;
  UpdateRootPageId(0);
}

/**
 * @brief
 * Insert constant key & value pair into leaf page
 * @return: true for insertion; false for modificwation
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction) {
  Page *leaf_page = FindLeafPage(key, false);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());

  if (leaf_node->Insert(key, value, comparator_) == -1) {
    /* unpin */
    buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), true);
    return false;
  }

  if (leaf_node->GetSize() < leaf_node->GetMaxSize()) {
    /* unpin */
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);  // should be done immediately
  } else {
    /* split and create a new node */
    /* NOTE: something should be updated here, the node can never be full */
    LeafPage *extra_node = Split<LeafPage>(leaf_node);
    extra_node->SetNextPageId(leaf_node->GetNextPageId());
    leaf_node->SetNextPageId(extra_node->GetPageId());

    /* insert the smallest key value into parent */
    InsertIntoParent(leaf_node, extra_node->KeyAt(0), extra_node, transaction);
  }
  return true;
}

/**
 * @brief
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREENTS_TYPE::Split(N *node) {
  /* the compiler cannot handle something exquisite, so I have to point out their types */
  if (node->IsLeafPage()) {
    LeafPage *curr_node = reinterpret_cast<LeafPage *>(node);
    LeafPage *extra_node = NewNode<LeafPage>(node->GetParentPageId(), IndexPageType::LEAF_PAGE);
    curr_node->MoveHalfTo(extra_node);
    N *res_node = reinterpret_cast<N *>(extra_node);
    return res_node;
  }

  InternalPage *curr_node = reinterpret_cast<InternalPage *>(node);
  InternalPage *extra_node = NewNode<InternalPage>(node->GetParentPageId(), IndexPageType::INTERNAL_PAGE);
  curr_node->MoveHalfTo(extra_node, buffer_pool_manager_);
  N *res_node = reinterpret_cast<N *>(extra_node);
  return res_node;
}

/**
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREENTS_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                         Transaction *transaction) {
  if (old_node->IsRootPage()) {
    InternalPage *root_node = NewNode<InternalPage>(INVALID_PAGE_ID, IndexPageType::INTERNAL_PAGE);
    page_id_t root_page_id = root_node->GetPageId();

    /* resolve some relationship */
    root_node->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id);
    new_node->SetParentPageId(root_page_id);
    root_page_id_ = root_page_id;
    UpdateRootPageId(0);

    /* unpin */
    buffer_pool_manager_->UnpinPage(root_page_id, true);
    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
    return;
  }
  page_id_t old_page_id = old_node->GetPageId();
  page_id_t new_page_id = new_node->GetPageId();
  Page *parent_page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  InternalPage *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());

  /* unpin */
  buffer_pool_manager_->UnpinPage(old_page_id, true);
  buffer_pool_manager_->UnpinPage(new_page_id, true);

  parent_node->InsertNodeAfter(old_page_id, key, new_page_id);
  if (parent_node->GetSize() == parent_node->GetMaxSize()) {
    /* still need to split */
    InternalPage *extra_node = Split<InternalPage>(parent_node);

    /* insert the smallest key value into parent */
    InsertIntoParent(parent_node, extra_node->KeyAt(0), extra_node, transaction);
  } else {
    /* unpin and it's modified! */
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
  }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/**
 * @brief
 * delete key & value pair associated with input key
 * remove an non-exist key is allowed
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::Remove(const KeyType &key, Transaction *transaction) {
  Page *leaf_page = FindLeafPage(key, false);  // leaf_page is pinned
  if (leaf_page == nullptr) {
    return false;
  }
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  if (leaf_node->RemoveAndDeleteRecord(key, comparator_) == -1) {
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), false);
    return false;
    //    throw std::runtime_error("Entry is not found. ");
  }
  CoalesceOrRedistribute(leaf_node, transaction);
  return true;
}

/**
 * @brief
 * try either coalescing or redistributing
 * @return: true means target leaf page should be deleted, false means no deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREENTS_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {
  /* Guarantee that only node is pinned is the beginning. */
  if (node->GetSize() >= node->GetMinSize()) {
    buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
    return true;
  }

  if (node->IsRootPage()) {
    return AdjustRoot(node);
  }

  /* try to coalesce with the previous child */
  Page *parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());  // parent_page is pinned
  InternalPage *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());
  int node_id = parent_node->ValueIndex(node->GetPageId());

  /* try to find the previous one, find the next one if failed */
  page_id_t neighbor_page_id = parent_node->ValueAt(!node_id ? 1 : node_id - 1);
  Page *neighbor_page = buffer_pool_manager_->FetchPage(neighbor_page_id);  // neighbor_page is pinned
  N *neighbor_node = reinterpret_cast<N *>(neighbor_page->GetData());

  /* take care that here is <, instead of <= */
  if (neighbor_node->GetSize() + node->GetSize() < node->GetMaxSize()) {
    /* coalesce */
    return Coalesce(&neighbor_node, &node, &parent_node, node_id, transaction);
  }
  /* redistribution */
  Redistribute(neighbor_node, node, !node_id ? 0 : 1);

  /* unpin */
  buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
  return false;
}

/**
 * @brief
 * Move all the key & value pairs from one page to its sibling page, namely merging
 * redistribute recursively if necessary.
 * @return  true means parent node should be deleted, false means no deletion happend
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREENTS_TYPE::Coalesce(N **neighbor_node, N **node,
                                 BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent, int index,
                                 Transaction *transaction) {
  // puts("Coalesce happens.");
  /* Guarantee that *neighbor_node, *node, and *parent are all pinned in the beginning. */
  if (!index) {
    /* make sure that neighbor_node is the predescssor of the node */
    std::swap(node, neighbor_node);
    index = 1;
  }

  KeyType middle_key = (*parent)->KeyAt(index);

  if (!(*node)->IsLeafPage()) {
    InternalPage *curr_node = reinterpret_cast<InternalPage *>(*node);
    InternalPage *prev_node = reinterpret_cast<InternalPage *>(*neighbor_node);
    curr_node->MoveAllTo(prev_node, middle_key, buffer_pool_manager_);
  } else {
    LeafPage *curr_node = reinterpret_cast<LeafPage *>(*node);
    LeafPage *prev_node = reinterpret_cast<LeafPage *>(*neighbor_node);
    curr_node->MoveAllTo(prev_node);
    prev_node->SetNextPageId(curr_node->GetNextPageId());
  }

  (*parent)->Remove(index);

  /* unpin */
  buffer_pool_manager_->UnpinPage((*neighbor_node)->GetPageId(), true);  // neighbor_node is unpinned
  buffer_pool_manager_->UnpinPage((*node)->GetPageId(), true);           // node is unpinned

  return CoalesceOrRedistribute(*parent, transaction);
}

/**
 * @brief
 * redistribute key & value pairs from one page to its sibling page
 * @param neighbor_node sibling page of input "node"
 * @param node input from method coalesceOrRedistribute()
 * @param index 0 for moving sibling page's first key & value pair into end of input "node"
 * @param index 1 for move sibling page's last key & value pair into head of input "node"
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREENTS_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
  /**
   * @brief
   * Format: 0: node neighbor_node; 1: neighbor_node node.
   * Guarantee that neighbor_node, node, and parent_node is pinned.
   */
  // puts("Redistribute happens.");
  page_id_t parent_page_id = node->GetParentPageId();
  Page *parent_page = buffer_pool_manager_->FetchPage(parent_page_id);  // parent_node is pinned again
  InternalPage *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());

  int array_id = parent_node->ValueIndex(!index ? neighbor_node->GetPageId() : node->GetPageId());
  KeyType middle_key = parent_node->KeyAt(array_id);

  if (neighbor_node->IsLeafPage()) {
    LeafPage *leaf_neighbor_node = reinterpret_cast<LeafPage *>(neighbor_node);
    LeafPage *leaf_node = reinterpret_cast<LeafPage *>(node);
    if (index == 0) {
      /* move sibling page's first key & value pair into end of input node */
      leaf_neighbor_node->MoveFirstToEndOf(leaf_node);
      parent_node->SetKeyAt(array_id, leaf_neighbor_node->KeyAt(0));
    } else {
      /* move sibling page's last key & value pair into head of input node */
      leaf_neighbor_node->MoveLastToFrontOf(leaf_node);
      parent_node->SetKeyAt(array_id, leaf_node->KeyAt(0));
    }
  } else {
    InternalPage *internal_neighbor_node = reinterpret_cast<InternalPage *>(neighbor_node);
    InternalPage *internal_node = reinterpret_cast<InternalPage *>(node);
    if (index == 0) {
      /* move sibling page's first key & value pair into end of input node */
      internal_neighbor_node->MoveFirstToEndOf(internal_node, middle_key, buffer_pool_manager_);
      parent_node->SetKeyAt(array_id, internal_neighbor_node->KeyAt(0));
    } else {
      /* move sibling page's last key & value pair into head of input node */
      internal_neighbor_node->MoveLastToFrontOf(internal_node, middle_key, buffer_pool_manager_);
      parent_node->SetKeyAt(array_id, internal_node->KeyAt(0));
    }
  }

  /* unpin */
  buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(neighbor_node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
}

/**
 * @brief
 * substitute the root due to removal
 * @return : true means root page should be deleted, false means no deletion
 * happend
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREENTS_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
  /* case 1: when you delete the last element in root page, but root page still has one last child */
  if (!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1) {
    InternalPage *root_node = reinterpret_cast<InternalPage *>(old_root_node);
    page_id_t child_page_id = root_node->RemoveAndReturnOnlyChild();

    /* unpin and delete */
    buffer_pool_manager_->UnpinPage(root_node->GetPageId(), true);
    buffer_pool_manager_->DeletePage(root_node->GetPageId());

    Page *child_page = buffer_pool_manager_->FetchPage(child_page_id);
    BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
    child_node->SetParentPageId(INVALID_PAGE_ID);

    root_page_id_ = child_node->GetPageId();

    /* unpin again */
    buffer_pool_manager_->UnpinPage(child_page->GetPageId(), true);

    UpdateRootPageId(0);

    return true;
  }

  /* case 2: when you delete the last element in whole b+ tree */
  if (old_root_node->IsLeafPage() && old_root_node->GetSize() == 0) {
    /* unpin and delete */
    buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), true);
    buffer_pool_manager_->DeletePage(old_root_node->GetPageId());

    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId(0);

    return true;
  }

  buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), true);
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/**
 * @brief
 * input parameter is void, find the leaftmost leaf page first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREENTS_TYPE::begin() {
  Page *leaf_page = FindLeafPage(KeyType(), true);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  return INDEXITERATOR_TYPE(leaf_node, 0, buffer_pool_manager_);
}

/**
 * @brief
 * input parameter is low key, find the leaf page that contains the input key first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREENTS_TYPE::Begin(const KeyType &key) {
  Page *leaf_page = FindLeafPage(key);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  int index = leaf_node->KeyIndex(key, comparator_);
  return INDEXITERATOR_TYPE(leaf_node, index, buffer_pool_manager_);
}

/**
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREENTS_TYPE::end() { return INDEXITERATOR_TYPE(nullptr, 0, buffer_pool_manager_); }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/**
 * @brief
 * find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREENTS_TYPE::FindLeafPage(const KeyType &key, bool leftMost, bool rightMost) {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return nullptr;
  }
  Page *next_page = buffer_pool_manager_->FetchPage(root_page_id_);

  BPlusTreePage *test_node = reinterpret_cast<BPlusTreePage *>(next_page->GetData());
  if (test_node->IsLeafPage()) {
    return next_page;
  }

  InternalPage *internal_node = reinterpret_cast<InternalPage *>(next_page->GetData());
  page_id_t last_page_id = root_page_id_;

  /* traverse down from root */
  while (true) {
    /* find the way down */
    page_id_t next_page_id = leftMost ? internal_node->ValueAt(0)
                                      : (rightMost ? internal_node->ValueAt(internal_node->GetSize() - 1)
                                                   : internal_node->Lookup(key, comparator_));

    /* prepare for next step */
    next_page = buffer_pool_manager_->FetchPage(next_page_id);
    BPlusTreePage *next_node = reinterpret_cast<BPlusTreePage *>(next_page->GetData());

    /* unpin */
    buffer_pool_manager_->UnpinPage(last_page_id, false);
    last_page_id = next_page_id;

    /* if it's a leaf node, then jump out */
    if (next_node->IsLeafPage()) {
      return next_page;
    }

    /* otherwise, it still in the internal tree, keep traversing */
    internal_node = reinterpret_cast<InternalPage *>(next_node);
  }
}

/**
 * @brief
 * update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @param insert_record default value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREENTS_TYPE::UpdateRootPageId(int insert_record) {
  HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page
    header_page->UpdateRecord(index_name_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/**
 * This function is for debug only, you don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREENTS_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    LeafPage *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    InternalPage *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

DECLARE(BPlusTreeNTS)

}  // namespace thomas

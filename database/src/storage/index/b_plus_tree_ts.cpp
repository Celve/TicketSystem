#include "storage/index/b_plus_tree_ts.h"

#include <chrono>
#include <string>
#include <thread>

#include "common/macros.h"
#include "storage/index/b_plus_tree_nts.h"
#include "storage/page/b_plus_tree_page.h"
#include "storage/page/header_page.h"

namespace thomas {
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREETS_TYPE::BPlusTreeTS(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                              int leaf_max_size, int internal_max_size)  // NOLINT
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  header_page->SearchRecord(index_name_, &root_page_id_);
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
  processed_ = 0;
}

/**
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::IsEmpty() const { return root_page_id_ == INVALID_PAGE_ID; }

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/**
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *transaction) {
  Page *leaf_page = CrabToLeaf(key, TransactionType::FIND, false, false, transaction);
  if (leaf_page == nullptr) {
    return false;
  }
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  ValueType value;
  bool flag = false;

  if (leaf_node->Lookup(key, &value, comparator_)) {
    result->push_back(value);
    flag = true;
  }

  /* unlatch and unpin */
  UnlatchPage(leaf_page, TransactionType::FIND);
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
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, const KeyComparator &new_comparator,
                                Transaction *transaction) {
  root_latch_.WLock();
  while (processed_ != 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  Page *leaf_page = CrabToLeaf(key, TransactionType::MULTIFIND, false, true, transaction);
  if (leaf_page == nullptr) {
    root_latch_.WUnlock();
    // transaction->Unlock();
    return false;
  }

  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  ValueType value;

  int index = leaf_node->KeyIndex(key, comparator_);

  auto clear_up = [&]() {
    UnlatchPage(leaf_page, TransactionType::MULTIFIND);
    buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), false);
  };

  while (true) {
    if (index != -1) {
      if (new_comparator(key, leaf_node->KeyAt(index))) {
        clear_up();
        break;
      }
      result->push_back(leaf_node->GetItem(index++).second);
    }
    if (index == -1 || index == leaf_node->GetSize()) {
      index = 0;
      /* get next page id */
      page_id_t next_page_id = leaf_node->GetNextPageId();

      /* unlatch and unpin */
      clear_up();

      if (next_page_id == INVALID_PAGE_ID) {
        break;
      }

      leaf_page = buffer_pool_manager_->FetchPage(next_page_id);
      leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
      LatchPage(leaf_page, TransactionType::MULTIFIND);
    }
  }
  root_latch_.WUnlock();
  // transaction->Unlock();
  return true;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction, bool isCalled) {
  root_latch_.WLock();
  if (!isCalled) {
    ++processed_;
    // transaction->Unlock();
  }
  if (IsEmpty()) {
    StartNewTree(key, value);
    --processed_;
    root_latch_.WUnlock();
    return true;
  }
  return InsertIntoLeaf(key, value, transaction);
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::OptimisticInsert(const KeyType &key, const ValueType &value, Transaction *transaction) {
  ++processed_;
  InsertState flag;
  TentativeInsert(key, value, flag, transaction);
  switch (flag) {
    case InsertState::SUCCESS:
      --processed_;
      return true;

    case InsertState::DUPLICATE_KEY:
      --processed_;
      return false;

    default:
      return Insert(key, value, transaction, true);
  }
}

/**
 * @brief
 * An optimistic try to insert, which means it only use read latch along the way.
 * @param key
 * @param value
 * @param transaction
 * @return An insert state indicates the outcome.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::TentativeInsert(const KeyType &key, const ValueType &value, InsertState &insert_state,
                                       Transaction *transaction) {
  Page *leaf_page = CrabToLeaf(key, TransactionType::OPTIMISTIC_INSERT, false, false, transaction);
  if (leaf_page == nullptr) {
    insert_state = InsertState::NO_ROOT;
    return;
  }

  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  insert_state = InsertState::SUCCESS;
  if (!leaf_node->IsInsertSafe()) {
    insert_state = InsertState::UNSAFE;
  } else if (leaf_node->Insert(key, value, comparator_) == -1) {
    insert_state = InsertState::DUPLICATE_KEY;
  }

  /* unlatch and unpin */
  UnlatchPage(leaf_page, TransactionType::OPTIMISTIC_INSERT);
  buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), insert_state == InsertState::SUCCESS);
}

/**
 * @brief
 * create a new page, which might be different types
 * @param parent_id
 * @return INDEX_TEMPLATE_ARGUMENTS*
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREETS_TYPE::NewNode(page_id_t parent_id, IndexPageType page_type) {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(&page_id);

  /* all frames in buffer pool are pinned */
  if (page == nullptr) {
    throw std::runtime_error("Cannot fetch a new page.");
  }

  N *node = reinterpret_cast<N *>(page->GetData());
  if (page_type == IndexPageType::INTERNAL_PAGE) {
    node->Init(page_id, parent_id, internal_max_size_);
  } else {
    node->Init(page_id, parent_id, leaf_max_size_);
  }
  node->SetPageType(page_type);
  node->SetSize(0);
  return node;
}

/**
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  LeafPage *node = NewNode<LeafPage>(INVALID_PAGE_ID, IndexPageType::LEAF_PAGE);

  /* insert */
  node->Insert(key, value, comparator_);

  page_id_t page_id = node->GetPageId();
  buffer_pool_manager_->UnpinPage(page_id, true);
  root_page_id_ = page_id;
  UpdateRootPageId(0);
}

/**
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immediately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction) {
  Page *leaf_page = CrabToLeaf(key, TransactionType::INSERT, false, true, transaction);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());

  /* the key is duplicate, which is not allowed */
  if (leaf_node->Insert(key, value, comparator_) == -1) {
    /* unlatch and unpin */
    ReleasePages(TransactionType::INSERT, transaction);
    UnlatchPage(leaf_page, TransactionType::INSERT);
    buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), true);
    --processed_;

    return false;
  }

  /* whether to split or not, according to the size */
  if (leaf_node->GetSize() >= leaf_node->GetMaxSize()) {
    /* split and create a new node */
    /* NOTE: something should be updated here, the node can never be full */
    LeafPage *extra_node = Split<LeafPage>(leaf_node);
    extra_node->SetNextPageId(leaf_node->GetNextPageId());
    leaf_node->SetNextPageId(extra_node->GetPageId());
    buffer_pool_manager_->FetchPage(leaf_page->GetPageId());
    /* insert the smallest key value into parent */
    InsertIntoParent(leaf_node, extra_node->KeyAt(0), extra_node, transaction);
  }
  UnlatchPage(leaf_page, TransactionType::INSERT);
  buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);  // should be done immediately
  ReleasePages(TransactionType::INSERT, transaction);
  --processed_;
  return true;
}

/**
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREETS_TYPE::Split(N *node) {
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
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                        Transaction *transaction) {
  if (old_node->IsRootPage()) {
    InternalPage *root_node = NewNode<InternalPage>(INVALID_PAGE_ID, IndexPageType::INTERNAL_PAGE);
    page_id_t root_page_id = root_node->GetPageId();

    /* resolve some relationship */
    root_node->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id);
    new_node->SetParentPageId(root_page_id);

    /* update the root_page_id */
    root_page_id_ = root_page_id;
    UpdateRootPageId(0);

    /* unlatch the root_pate_id */
    root_latch_.WUnlock();

    /* unlatch and unpin */
    buffer_pool_manager_->UnpinPage(root_page_id, true);
    // UnlatchNode(old_node, LatchMode::WRITE);
    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);

    return;
  }

  page_id_t old_page_id = old_node->GetPageId();
  page_id_t new_page_id = new_node->GetPageId();
  Page *parent_page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  InternalPage *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());

  /* unlatch and unpin */
  // UnlatchNode(old_node, LatchMode::WRITE);
  buffer_pool_manager_->UnpinPage(old_page_id, true);
  buffer_pool_manager_->UnpinPage(new_page_id, true);

  parent_node->InsertNodeAfter(old_page_id, key, new_page_id);
  if (parent_node->GetSize() == parent_node->GetMaxSize()) {
    /* still need to split */
    InternalPage *extra_node = Split<InternalPage>(parent_node);

    /* insert the smallest key value into parent */
    InsertIntoParent(parent_node, extra_node->KeyAt(0), extra_node, transaction);
  } else {
    /* unlatch and unpin the modified page! */
    // UnlatchPage(parent_page, LatchMode::WRITE);
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
  }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/**
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::Remove(const KeyType &key, Transaction *transaction, bool isCalled) {
  if (!isCalled) {
    ++processed_;
  }
  Page *leaf_page = CrabToLeaf(key, TransactionType::DELETE, false, false, transaction);  // leaf_page is pinned
  if (leaf_page == nullptr) {
    --processed_;
    return false;
  }

  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  bool flag = false;
  if (leaf_node->RemoveAndDeleteRecord(key, comparator_) != -1) {
    CoalesceOrRedistribute(leaf_node, transaction);
    flag = true;
  } else {
    /* don't forget to do such a job */
    buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), false);
  }
  ReleasePages(TransactionType::DELETE, transaction);
  UnlatchPage(leaf_page, TransactionType::DELETE);
  DeletePages(transaction);
  --processed_;
  return flag;
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::OptimisticRemove(const KeyType &key, Transaction *transaction) {
  ++processed_;
  DeleteState flag = DeleteState::UNSAFE;
  TentativeRemove(key, flag, transaction);
  switch (flag) {
    case DeleteState::SUCCESS:
      --processed_;
      return true;

    case DeleteState::NO_ENTRY:
      --processed_;
      return false;

    case DeleteState::PAGE_FAULT:
      --processed_;
      return false;

    case DeleteState::UNSAFE:
      return Remove(key, transaction, true);
  }
  /* it should never be here */
  return false;
}

INDEX_TEMPLATE_ARGUMENTS void BPLUSTREETS_TYPE::TentativeRemove(const KeyType &key, DeleteState &delete_state,
                                                                Transaction *transaction) {
  Page *leaf_page = CrabToLeaf(key, TransactionType::OPTIMISTIC_DELETE, false, false, transaction);
  if (leaf_page == nullptr) {
    delete_state = DeleteState::PAGE_FAULT;
    return;
  }

  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  delete_state = DeleteState::SUCCESS;
  if (!leaf_node->IsDeleteSafe()) {
    delete_state = DeleteState::UNSAFE;
  } else if (leaf_node->RemoveAndDeleteRecord(key, comparator_) == -1) {
    delete_state = DeleteState::NO_ENTRY;
  }

  /* unlatch and unpin */
  UnlatchPage(leaf_page, TransactionType::OPTIMISTIC_DELETE);
  buffer_pool_manager_->UnpinPage(leaf_node->GetPageId(), delete_state == DeleteState::SUCCESS);
}

/**
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREETS_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {
  /* Guarantee that only node is pinned is the beginning. */
  if (node->GetSize() >= node->GetMinSize()) {
    buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
    return true;
  }

  if (node->IsRootPage()) {
    return AdjustRoot(node, transaction);
  }

  /* try to coalesce with the previous child */
  Page *parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());  // parent_page is pinned
  InternalPage *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());
  int node_id = parent_node->ValueIndex(node->GetPageId());

  /* try to find the previous one, find the next one if failed */
  page_id_t neighbor_page_id = parent_node->ValueAt(!node_id ? 1 : node_id - 1);
  Page *neighbor_page = buffer_pool_manager_->FetchPage(neighbor_page_id);  // neighbor_page is pinned
  LatchPage(neighbor_page, TransactionType::DELETE);
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
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion
 * happend
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREETS_TYPE::Coalesce(N **neighbor_node, N **node,
                                BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent, int index,
                                Transaction *transaction) {
  /* Guarantee that *neighbor_node, *node, and *parent are all pinned in the beginning. */
  bool is_swap = false;
  if (!index) {
    /* make sure that neighbor_node is the predescssor of the node */
    is_swap = true;
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

  /* unlatch and unpin and delete */
  UnlatchNode<N>(is_swap ? *node : *neighbor_node, TransactionType::DELETE);
  buffer_pool_manager_->UnpinPage((*neighbor_node)->GetPageId(), true);  // neighbor_node is unpinned
  buffer_pool_manager_->UnpinPage((*node)->GetPageId(), true);           // node is unpinned
  transaction->AddIntoDeletedPageSet((*node)->GetPageId());

  return CoalesceOrRedistribute(*parent, transaction);
}

/**
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREETS_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
  /**
   * @brief
   * Format: 0: node neighbor_node; 1: neighbor_node node.
   * Guarantee that neighbor_node, node, and parent_node is pinned.
   */
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

  /* unlatch and unpin */
  buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
  UnlatchNode<N>(neighbor_node, TransactionType::DELETE);
  buffer_pool_manager_->UnpinPage(neighbor_node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
}

/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happend
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREETS_TYPE::AdjustRoot(BPlusTreePage *old_root_node, Transaction *transaction) {
  /* case 1: when you delete the last element in root page, but root page still has one last child */
  if (!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1) {
    InternalPage *root_node = reinterpret_cast<InternalPage *>(old_root_node);
    page_id_t child_page_id = root_node->RemoveAndReturnOnlyChild();

    /* unpin and delete */
    buffer_pool_manager_->UnpinPage(root_node->GetPageId(), true);
    transaction->AddIntoDeletedPageSet(root_node->GetPageId());

    Page *child_page = buffer_pool_manager_->FetchPage(child_page_id);
    BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
    child_node->SetParentPageId(INVALID_PAGE_ID);

    /* unpin again */
    buffer_pool_manager_->UnpinPage(child_page->GetPageId(), true);

    /* update the root_page_id */
    root_page_id_ = child_node->GetPageId();
    UpdateRootPageId(0);

    /* unlatch the root_page_id */
    /* TODO: I think the root latch should not be unlocked here, because it would be released in the
     * release page function */
    // root_latch_.WUnlock();

    return true;
  }

  /* case 2: when you delete the last element in whole b+ tree */
  if (old_root_node->IsLeafPage() && old_root_node->GetSize() == 0) {
    /* unpin and delete */
    buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), true);
    transaction->AddIntoDeletedPageSet(old_root_node->GetPageId());

    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId(0);
    /* TODO: I think the root latch should not be unlocked here, because it would be released in the
     * releasepage function */
    // root_latch_.WUnlock();

    return true;
  }

  buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), true);
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/**
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREETS_TYPE::begin() {
  Page *leaf_page = FindLeafPage(KeyType(), true);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  UnlatchPage(leaf_page, TransactionType::FIND);
  return INDEXITERATOR_TYPE(leaf_node, 0, buffer_pool_manager_);
}

/**
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREETS_TYPE::Begin(const KeyType &key) {
  Page *leaf_page = FindLeafPage(key);
  LeafPage *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  int index = leaf_node->KeyIndex(key, comparator_);
  UnlatchPage(leaf_page, TransactionType::FIND);
  return INDEXITERATOR_TYPE(leaf_node, index, buffer_pool_manager_);
}

/**
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREETS_TYPE::end() { return INDEXITERATOR_TYPE(nullptr, 0, buffer_pool_manager_); }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/

/**
 * @brief
 * Find the leaf page with latch alongside.
 * @param key
 * @param latch_mode
 * @param leftMost
 * @return the target leaf page with right latch.
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREETS_TYPE::CrabToLeaf(const KeyType &key, TransactionType transaction_type, bool leftMost,
                                   bool rootLatched, Transaction *transaction) {
  /* if it's insertion, the root is pre-locked, otherwise the root should be locked */
  if (!rootLatched) {
    LockRoot(transaction_type);
  }

  if (root_page_id_ == INVALID_PAGE_ID) {
    UnlockRoot(transaction_type);
    return nullptr;
  }

  Page *last_page = buffer_pool_manager_->FetchPage(root_page_id_);

  /* avoid repeated read latch */
  LatchPage(last_page, transaction_type);

  BPlusTreePage *test_node = reinterpret_cast<BPlusTreePage *>(last_page->GetData());
  if (test_node->IsLeafPage()) {
    return last_page;
  }

  InternalPage *last_node = reinterpret_cast<InternalPage *>(last_page->GetData());
  page_id_t last_page_id = root_page_id_;

  /* traverse down from root */
  while (true) {
    /* find the way down */
    page_id_t next_page_id = leftMost ? last_node->ValueAt(0) : last_node->Lookup(key, comparator_);

    /* prepare for next step */
    Page *next_page = buffer_pool_manager_->FetchPage(next_page_id);
    BPlusTreePage *next_node = reinterpret_cast<BPlusTreePage *>(next_page->GetData());

    LatchPage(next_page, transaction_type);
    if (transaction_type == TransactionType::OPTIMISTIC_INSERT ||
        transaction_type == TransactionType::OPTIMISTIC_DELETE || transaction_type == TransactionType::FIND ||
        transaction_type == TransactionType::MULTIFIND) {
      /* unlatch and unpin */
      UnlatchPage(last_page, transaction_type);
      buffer_pool_manager_->UnpinPage(last_page_id, false);
    } else {
      /* add to transaction page list */
      transaction->AddIntoPageSet(last_page);
      if ((transaction_type == TransactionType::INSERT && next_node->IsInsertSafe()) ||
          (transaction_type == TransactionType::DELETE && next_node->IsDeleteSafe())) {
        ReleasePages(transaction_type, transaction);
      }
    }

    /* switch the last and the next */
    last_page_id = next_page_id;
    last_page = next_page;

    /* if it's a leaf node, then jump out */
    if (next_node->IsLeafPage()) {
      return next_page;
    }

    /* otherwise, it still in the internal tree, keep traversing */
    last_node = reinterpret_cast<InternalPage *>(next_node);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::UnlatchPage(Page *page, TransactionType transaction_type) {
  BPlusTreePage *node = reinterpret_cast<BPlusTreePage *>(page->GetData());
  if (node->IsRootPage()) {
    UnlockRoot(transaction_type);
  }

  if (transaction_type == TransactionType::INSERT || transaction_type == TransactionType::DELETE ||
      ((transaction_type == TransactionType::OPTIMISTIC_INSERT ||
        transaction_type == TransactionType::OPTIMISTIC_DELETE) &&
       node->IsLeafPage())) {
    page->WUnlatch();
  } else {
    page->RUnlatch();
  }
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREETS_TYPE::UnlatchNode(N *node, TransactionType transaction_type) {
  page_id_t page_id = node->GetPageId();
  Page *page = buffer_pool_manager_->FetchPage(page_id);
  UnlatchPage(page, transaction_type);
  buffer_pool_manager_->UnpinPage(page_id, false);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::LatchPage(Page *page, TransactionType transaction_type) {
  BPlusTreePage *node = reinterpret_cast<BPlusTreePage *>(page->GetData());
  if ((transaction_type == TransactionType::INSERT || transaction_type == TransactionType::DELETE) ||
      ((transaction_type == TransactionType::OPTIMISTIC_INSERT ||
        transaction_type == TransactionType::OPTIMISTIC_DELETE) &&
       node->IsLeafPage())) {
    page->WLatch();
  } else {
    page->RLatch();
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::ReleasePages(TransactionType transaction_type, Transaction *transaction) {
  for (Page *page : *transaction->GetPageSet()) {
    UnlatchPage(page, transaction_type);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  }
  transaction->GetPageSet()->clear();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::DeletePages(Transaction *transaction) {
  for (Page *page : *transaction->GetPageSet()) {
    buffer_pool_manager_->DeletePage(page->GetPageId());
  }
  transaction->GetDeletedPageSet()->clear();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::LockRoot(TransactionType transaction_type) {
  if (transaction_type == TransactionType::OPTIMISTIC_INSERT ||
      transaction_type == TransactionType::OPTIMISTIC_DELETE || transaction_type == TransactionType::FIND ||
      transaction_type == TransactionType::MULTIFIND) {
    root_latch_.RLock();
  } else {
    root_latch_.WLock();
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::UnlockRoot(TransactionType transaction_type) {
  if (transaction_type == TransactionType::OPTIMISTIC_INSERT ||
      transaction_type == TransactionType::OPTIMISTIC_DELETE || transaction_type == TransactionType::FIND) {
    root_latch_.RUnlock();
  } else if (transaction_type != TransactionType::MULTIFIND) {
    root_latch_.WUnlock();
  }
}

/**
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREETS_TYPE::FindLeafPage(const KeyType &key, bool leftMost, Transaction *transaction) {
  return CrabToLeaf(key, TransactionType::FIND, leftMost, false, transaction);
}

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::UpdateRootPageId(int insert_record) {
  HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page and make sure that the root is always pinned
    page_id_t last_root_page_id;
    header_page->SearchRecord(index_name_, &last_root_page_id);
    if (last_root_page_id != -1) {
      buffer_pool_manager_->UnpinPage(last_root_page_id, false);
    }
    header_page->UpdateRecord(index_name_, root_page_id_);
    if (root_page_id_ != -1) {
      buffer_pool_manager_->FetchPage(root_page_id_);
    }
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/**
 * This method is used for debug only, You don't  need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 * @param out
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREETS_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {  // NOLINT
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    LeafPage *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    InternalPage *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
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
void BPLUSTREETS_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
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

DECLARE(BPlusTreeTS);

}  // namespace thomas

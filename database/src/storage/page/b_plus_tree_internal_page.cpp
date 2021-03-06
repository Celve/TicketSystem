#include "storage/page/b_plus_tree_internal_page.h"

#include <iostream>
#include <sstream>

#include "common/macros.h"
#include "storage/page/b_plus_tree_page.h"

namespace thomas {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/**
 * @brief
 * init method after creating a new internal page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetSize(0);
}

/**
 * @brief
 * Helper method to get the key by index
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const { return array[index].first; }

/**
 * @brief
 * Helper method to get the key by index
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) { array[index].first = key; }

/**
 * @brief
 * helper method to find and return array index by value
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const {
  for (int i = 0; i < GetSize(); ++i) {
    if (array[i].second == value) {
      return i;
    }
  }
  /* case that cannot find it */
  return -1;
}

/**
 * @brief
 * Helper method to get the value by index
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const { return array[index].second; }

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/**
 * @brief
 * find and return the child pointer by given key
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key, const KeyComparator &comparator) const {
  int l = 1;
  int r = GetSize() - 1;
  int child_index = GetSize() - 1;

  while (l <= r) {
    int mid = (l + r) >> 1;
    if (comparator(key, KeyAt(mid)) < 0) {
      child_index = mid - 1;
      r = mid - 1;
    } else {
      l = mid + 1;
    }
  }
  return array[child_index].second;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * @brief
 * populate new root page with old_value + new_key & new_value
 * page, you should create a new root page and populate its elements.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(const ValueType &old_value, const KeyType &new_key,
                                                     const ValueType &new_value) {
  array[0].second = old_value;
  array[1] = MappingType{new_key, new_value};
  SetSize(2);
}
/**
 * @brief
 * insert new_key & new_value pair right after the pair with its value == old_value
 * @return: new size after insertion, -1 for invalid
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(const ValueType &old_value, const KeyType &new_key,
                                                    const ValueType &new_value) {
  if (GetSize() == GetMaxSize()) {
    return -1;
  }

  int index = ValueIndex(old_value);

  /* cannot find it */
  if (index == -1) {
    return -1;
  }

  /* insert */
  for (int i = GetSize(); i > index + 1; --i) {
    swap(array[i], array[i - 1]);
  }
  array[index + 1] = MappingType{new_key, new_value};
  IncreaseSize(1);
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/**
 * @brief
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(BPlusTreeInternalPage *recipient,
                                                BufferPoolManager *buffer_pool_manager) {
  /* todo: when to use the buffer_pool_manager */
  int new_this_size = GetMinSize();
  int new_recipient_size = GetSize() - new_this_size;

  /* transfer, notice that the recipient should be an empty node */
  recipient->CopyNFrom(array + new_this_size, new_recipient_size, buffer_pool_manager);

  /* modify the size */
  SetSize(new_this_size);
  recipient->SetSize(new_recipient_size);
}

/**
 * @brief
 * copy size from given items
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyNFrom(MappingType *items, int size, BufferPoolManager *buffer_pool_manager) {
  /* take care that the it might overflow */
  if (GetSize() + size > GetMaxSize()) {
    throw std::runtime_error("Page overflows.");
  }
  std::copy(items, items + size, array + GetSize());

  /* update its child pages */
  for (int i = GetSize(); i < GetSize() + size; ++i) {
    Page *child_page = buffer_pool_manager->FetchPage(ValueAt(i));
    BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
    child_node->SetParentPageId(GetPageId());
    buffer_pool_manager->UnpinPage(ValueAt(i), true);
  }

  IncreaseSize(size);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/**
 * @brief
 * Remove the key & value pair in internal page by index
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  for (int i = index + 1; i < GetSize(); ++i) {
    swap(array[i - 1], array[i]);
  }
  IncreaseSize(-1);
}

/**
 * @brief
 * Remove the only key & value pair in internal page and return the value
 * NOTE: this function should be called only when the last child of tree in deleted
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild() {
  /* it means that the root page should be deleted */
  SetSize(0);
  return ValueAt(0);
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/**
 * @brief
 * remove all of key & value pairs from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                               BufferPoolManager *buffer_pool_manager) {
  /* avoid corner case */
  if (recipient->GetSize() + GetSize() > GetMaxSize()) {
    throw std::runtime_error("Page overflows.");
  }

  /* copy(l, r, d) copy [l, r) to destination d */
  std::copy(array, array + GetSize(), recipient->array + recipient->GetSize());
  recipient->SetKeyAt(recipient->GetSize(), middle_key);

  for (int i = recipient->GetSize(); i < GetSize() + recipient->GetSize(); ++i) {
    Page *child_page = buffer_pool_manager->FetchPage(recipient->ValueAt(i));
    BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
    child_node->SetParentPageId(recipient->GetPageId());
    buffer_pool_manager->UnpinPage(recipient->ValueAt(i), true);
  }
  recipient->IncreaseSize(GetSize());
  SetSize(0);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/**
 * @brief
 * Remove the first key & value pair from this page to tail of "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                      BufferPoolManager *buffer_pool_manager) {
  recipient->CopyLastFrom(MappingType{middle_key, ValueAt(0)}, buffer_pool_manager);
  Remove(0);
}

/**
 * @brief
 * insert key & value pair in the last
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyLastFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  /* insert and maintain basic information */
  array[GetSize()] = pair;
  IncreaseSize(1);

  /* change the parent id */
  Page *child_page = buffer_pool_manager->FetchPage(ValueAt(GetSize() - 1));
  BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
  child_node->SetParentPageId(GetPageId());
  buffer_pool_manager->UnpinPage(ValueAt(GetSize() - 1), true);
}

/**
 * @brief
 * Remove the last key & value pair from this page to head of "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                       BufferPoolManager *buffer_pool_manager) {
  recipient->SetKeyAt(0, middle_key);
  recipient->CopyFirstFrom(array[GetSize() - 1], buffer_pool_manager);
  IncreaseSize(-1);
}

/**
 * @brief
 * insert key & value pair in the last
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyFirstFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  /* insert and maintain basic information */
  for (int i = GetSize(); i >= 1; --i) {
    std::swap(array[i], array[i - 1]);
  }
  array[0] = pair;
  IncreaseSize(1);

  /* change the parent id */
  Page *child_page = buffer_pool_manager->FetchPage(ValueAt(0));
  BPlusTreePage *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
  child_node->SetParentPageId(GetPageId());
  buffer_pool_manager->UnpinPage(ValueAt(0), true);
}

DECLARE_BPLUSTREE_INTERNALPAGE(BPlusTreeInternalPage)

}  // namespace thomas

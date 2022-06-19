#include "storage/page/b_plus_tree_leaf_page.h"

#include <sstream>

#include "common/macros.h"
#include "storage/page/b_plus_tree_page.h"

namespace thomas {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {  // NOLINT
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);
  SetNextPageId(INVALID_PAGE_ID);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
page_id_t B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/**
 * Helper method to find the first index i so that array[i].first >= key
 * NOTE: This method is only used when generating index iterator
 * @return -1 if not find
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &comparator) const {
  int l = 0;
  int r = GetSize() - 1;
  int index = -1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (comparator(KeyAt(mid), key) >= 0) {
      r = mid - 1;
      index = mid;
    } else {
      l = mid + 1;
    }
  }
  return index;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const { return array[index].first; }

/**
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a array offset)
 * NOTE: Be aware of invalid index
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &B_PLUS_TREE_LEAF_PAGE_TYPE::GetItem(int index) { return array[index]; }

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * Insert key & value pair into leaf page ordered by key
 * @return page size after insertion; -1 for duplicate key
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator) {
  if (GetSize() == GetMaxSize()) {
    return -1;
  }

  int index = KeyIndex(key, comparator);
  if (index == -1) {
    /* none is bigger */
    array[GetSize()] = MappingType{key, value};
    IncreaseSize(1);
    return GetSize();
  }

  /* duplicate key, but I just want to modify it */
  if (!comparator(key, KeyAt(index))) {
    array[index] = MappingType{key, value};
    return -1;
  }

  /* shift */
  for (int i = GetSize() - 1; i >= index; --i) {
    swap(array[i], array[i + 1]);
  }
  array[index] = MappingType{key, value};
  IncreaseSize(1);
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveHalfTo(BPlusTreeLeafPage *recipient) {
  int new_this_size = GetMinSize();
  int new_recipient_size = GetSize() - new_this_size;

  recipient->CopyNFrom(array + new_this_size, new_recipient_size);

  SetSize(new_this_size);
  recipient->SetSize(new_recipient_size);
}

/*
 * Copy starting from items, and copy {size} number of elements into me.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyNFrom(MappingType *items, int size) {
  /* take care that the it might overflow */
  std::copy(items, items + size, array + GetSize());

  IncreaseSize(size);
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
INDEX_TEMPLATE_ARGUMENTS
bool B_PLUS_TREE_LEAF_PAGE_TYPE::Lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const {
  int index = KeyIndex(key, comparator);
  if (index == -1 || comparator(KeyAt(index), key)) {
    return false;
  }

  *value = array[index].second;
  return true;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * exist, perform deletion, otherwise return immediately.
 * NOTE: store key&value pair continuously after deletion
 * @return   page size after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator) {
  int index = KeyIndex(key, comparator);
  if (index == -1 || comparator(KeyAt(index), key)) {
    return -1;
  }

  Remove(index);
  return GetSize();
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Remove(int index) {
  for (int i = index + 1; i < GetSize(); ++i) {
    swap(array[i - 1], array[i]);
  }
  IncreaseSize(-1);
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page. Don't forget
 * to update the next_page id in the sibling page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient) {
  recipient->CopyNFrom(array, GetSize());
  SetSize(0);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeLeafPage *recipient) {
  recipient->CopyLastFrom(array[0]);
  Remove(0);
}

/*
 * Copy the item into the end of my item list. (Append item to my array)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyLastFrom(const MappingType &item) {
  array[GetSize()] = item;
  IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeLeafPage *recipient) {
  IncreaseSize(-1);
  recipient->CopyFirstFrom(array[GetSize()]);
}

/*
 * Insert item at the front of my items. Move items accordingly.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyFirstFrom(const MappingType &item) {
  for (int i = GetSize() - 1; i >= 0; --i) {
    swap(array[i], array[i + 1]);
  }
  array[0] = item;
  IncreaseSize(1);
}

DECLARE(BPlusTreeLeafPage)

}  // namespace thomas

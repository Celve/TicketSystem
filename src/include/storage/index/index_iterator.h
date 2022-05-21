/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace thomas {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

 public:
  // you may define your own constructor based on your member variables
  IndexIterator(LeafPage *ptr, int index, BufferPoolManager *buffer_pool_manager);
  ~IndexIterator();

  bool isEnd();

  const MappingType &operator*();

  IndexIterator &operator++();

  bool operator==(const IndexIterator &itr) const { return ptr_ == itr.ptr_ && index_ == itr.index_; }

  bool operator!=(const IndexIterator &itr) const { return ptr_ != itr.ptr_ || index_ != itr.index_; }

 private:
  LeafPage *ptr_;
  int index_;
  BufferPoolManager *buffer_pool_manager_;
};

}  // namespace thomas

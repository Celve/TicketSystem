/**
 * index_iterator.cpp
 */
#include <cassert>

#include "storage/index/index_iterator.h"

namespace thomas {

/**
 * @brief
 * Presume that ptr is pinned.
 * @param ptr
 * @param index
 * @param buffer_pool_manager
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(LeafPage *ptr, int index, BufferPoolManager *buffer_pool_manager)
    : ptr_(ptr), index_(index), buffer_pool_manager_(buffer_pool_manager) {}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator() {
  if (ptr_ != nullptr) {
    buffer_pool_manager_->UnpinPage(ptr_->GetPageId(), false);
  }
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::isEnd() { return ptr_ == nullptr; }

INDEX_TEMPLATE_ARGUMENTS
const MappingType &INDEXITERATOR_TYPE::operator*() { return ptr_->GetItem(index_); }

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
  ++index_;
  if (index_ == ptr_->GetSize()) {
    index_ = 0;
    page_id_t next_page_id = ptr_->GetNextPageId();

    /* please remember to unpin */
    buffer_pool_manager_->UnpinPage(ptr_->GetPageId(), false);

    /* reach the end */
    if (next_page_id == INVALID_PAGE_ID) {
      ptr_ = nullptr;
      return *this;
    }

    Page *next_page = buffer_pool_manager_->FetchPage(next_page_id);
    ptr_ = reinterpret_cast<LeafPage *>(next_page->GetData());
  }
  return *this;
}

template class IndexIterator<FixedString<48>, size_t, FixedStringComparator<48>>;
template class IndexIterator<MixedStringInt<64>, size_t, MixedStringIntForMixedComparator<64>>;

}  // namespace thomas

#pragma once

#include "concurrency/transaction.h"
#include "storage/index/index_iterator.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/b_plus_tree_leaf_page.h"
#include "type/string.h"
#include "type/string_any.h"

namespace thomas {

#define BPLUSTREETS_TYPE BPlusTreeTS<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeTS {
  using InternalPage = BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator>;
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;
  enum class TransactionType { OPTIMISTIC_INSERT, OPTIMISTIC_DELETE, FIND, MULTIFIND, INSERT, DELETE };
  enum class InsertState { SUCCESS, DUPLICATE_KEY, NO_ROOT, UNSAFE };
  enum class DeleteState { SUCCESS, NO_ENTRY, PAGE_FAULT, UNSAFE };

 public:
  explicit BPlusTreeTS(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                       int leaf_max_size = LEAF_PAGE_SIZE, int internal_max_size = INTERNAL_PAGE_SIZE);

  // Returns true if this B+ tree has no keys and values.
  bool IsEmpty() const;

  // Insert a key-value pair into this B+ tree.
  bool Insert(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr, bool isCalled = false);

  // Insert a key-alue pair into this B+ tree. It's faster. But it might get the wrong answer.
  bool OptimisticInsert(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr);

  // Remove a key and its value from this B+ tree.
  void Remove(const KeyType &key, Transaction *transaction = nullptr, bool isCalled = false);

  // Remove a key and its value from this B+ tree. It's fater. But it might get the wrong answer.
  void OptimisticRemove(const KeyType &key, Transaction *transaction = nullptr);

  // return the value associated with a given key
  bool GetValue(const KeyType &key, vector<ValueType> *result, Transaction *transaction = nullptr);

  // return the value that equal to a given key using the given rule
  bool GetValue(const KeyType &key, vector<ValueType> *result, const KeyComparator &new_comparator,
                Transaction *transaction = nullptr);

  // index iterator
  INDEXITERATOR_TYPE begin();  // NOLINT
  INDEXITERATOR_TYPE Begin(const KeyType &key);
  INDEXITERATOR_TYPE end();  // NOLINT

  void Print(BufferPoolManager *bpm) {
    ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm);
  }

  // expose for test purpose
  Page *FindLeafPage(const KeyType &key, bool leftMost = false, Transaction *transaction = nullptr);

 private:
  template <typename N>
  N *NewNode(page_id_t parent_id, IndexPageType page_type);

  void StartNewTree(const KeyType &key, const ValueType &value);

  void TentativeInsert(const KeyType &key, const ValueType &value, InsertState &insert_state,
                       Transaction *transaction = nullptr);

  bool InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr);

  void InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                        Transaction *transaction = nullptr);

  void TentativeRemove(const KeyType &key, DeleteState &delete_state, Transaction *transaction);

  Page *CrabToLeaf(const KeyType &key, TransactionType transaction_type, bool leafMost = false,
                   bool rootLatched = false, bool isLocked = true, Transaction *transaction = nullptr);

  void ReleasePages(TransactionType transaction_type, Transaction *transaction);

  void DeletePages(Transaction *transaction);

  void UnlatchPage(Page *page, TransactionType transaction_type);

  void LatchPage(Page *page, TransactionType transaction_type);

  void LockRoot(TransactionType transaction_type);

  void UnlockRoot(TransactionType transaction_type);

  template <typename N>
  void UnlatchNode(N *node, TransactionType transaction_type);

  template <typename N>
  N *Split(N *node);

  template <typename N>
  bool CoalesceOrRedistribute(N *node, Transaction *transaction = nullptr);

  template <typename N>
  bool Coalesce(N **neighbor_node, N **node, BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent,
                int index, Transaction *transaction = nullptr);

  template <typename N>
  void Redistribute(N *neighbor_node, N *node, int index);

  bool AdjustRoot(BPlusTreePage *node, Transaction *transaction = nullptr);

  void UpdateRootPageId(int insert_record = 0);

  void ToString(BPlusTreePage *page, BufferPoolManager *bpm) const;

  // member variable
  std::string index_name_;
  page_id_t root_page_id_;
  ReaderWriterLatch root_latch_;
  BufferPoolManager *buffer_pool_manager_;
  KeyComparator comparator_;
  int leaf_max_size_;
  int internal_max_size_;
};

}  // namespace thomas

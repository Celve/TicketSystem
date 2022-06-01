#ifndef TICKETSYSTEM_BPLUSTREE_H
#define TICKETSYSTEM_BPLUSTREE_H
// the license is included in the source file

template <typename N>
class BPlusTree {
    using InternalPage = BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator>;
    using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

public:
    explicit BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                       int leaf_max_size = LEAF_PAGE_SIZE, int internal_max_size = INTERNAL_PAGE_SIZE);

    bool IsEmpty() const;

    bool Insert(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr);

    void Remove(const KeyType &key, Transaction *transaction = nullptr);

    bool GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *transaction = nullptr);

    Page *FindLeafPage(const KeyType &key, bool leftMost = false);

private:
    N *NewNode(page_id_t parent_id);

    void StartNewTree(const KeyType &key, const ValueType &value);

    bool InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr);

    void InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                          Transaction *transaction = nullptr);

    Page *ReachLeafPage(const KeyType &key);

    N *Split(N *node);

    bool CoalesceOrRedistribute(N *node, Transaction *transaction = nullptr);

    bool Coalesce(N **neighbor_node, N **node, BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent,
                  int index, Transaction *transaction = nullptr);

    void Redistribute(N *neighbor_node, N *node, int index);

    bool AdjustRoot(BPlusTreePage *node);

    void UpdateRootPageId(int insert_record = 0);

    std::string index_name_;
    page_id_t root_page_id_;
    BufferPoolManager *buffer_pool_manager_;
    KeyComparator comparator_;
    int leaf_max_size_;
    int internal_max_size_;
};

class BPlusTreeInternalPage : public BPlusTreePage {
public:
    void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = INTERNAL_PAGE_SIZE);

    KeyType KeyAt(int index) const;
    void SetKeyAt(int index, const KeyType &key);
    int ValueIndex(const ValueType &value) const;
    ValueType ValueAt(int index) const;

    ValueType Lookup(const KeyType &key, const KeyComparator &comparator) const;
    void PopulateNewRoot(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);
    int InsertNodeAfter(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);
    void Remove(int index);
    int Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);
    ValueType RemoveAndReturnOnlyChild();

    void MoveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middle_key, BufferPoolManager *buffer_pool_manager);
    void MoveHalfTo(BPlusTreeInternalPage *recipient, BufferPoolManager *buffer_pool_manager);
    void MoveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                          BufferPoolManager *buffer_pool_manager);
    void MoveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                           BufferPoolManager *buffer_pool_manager);

private:
    void CopyNFrom(MappingType *items, int size, BufferPoolManager *buffer_pool_manager);
    void CopyLastFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager);
    void CopyFirstFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager);
    MappingType array[0];
};

class BPlusTreeLeafPage : public BPlusTreePage {
public:
    void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = LEAF_PAGE_SIZE);
    page_id_t GetNextPageId() const;
    void SetNextPageId(page_id_t next_page_id);
    KeyType KeyAt(int index) const;
    int KeyIndex(const KeyType &key, const KeyComparator &comparator) const;
    const MappingType &GetItem(int index);

    int Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);
    bool Lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const;
    int RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator);
    void Remove(int index);

    void MoveHalfTo(BPlusTreeLeafPage *recipient);
    void MoveAllTo(BPlusTreeLeafPage *recipient);
    void MoveFirstToEndOf(BPlusTreeLeafPage *recipient);
    void MoveLastToFrontOf(BPlusTreeLeafPage *recipient);

private:
    void CopyNFrom(MappingType *items, int size);
    void CopyLastFrom(const MappingType &item);
    void CopyFirstFrom(const MappingType &item);
    page_id_t next_page_id_;
    MappingType array[0];
};

class BufferPoolManager {
public:
    BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager = nullptr);

    ~BufferPoolManager();

    Page *FetchPage(page_id_t page_id, bufferpool_callback_fn callback = nullptr);

    bool UnpinPage(page_id_t page_id, bool is_dirty, bufferpool_callback_fn callback = nullptr);

    bool FlushPage(page_id_t page_id, bufferpool_callback_fn callback = nullptr);

    Page *NewPage(page_id_t *page_id, bufferpool_callback_fn callback = nullptr);

    bool DeletePage(page_id_t page_id, bufferpool_callback_fn callback = nullptr);

    void FlushAllPages(bufferpool_callback_fn callback = nullptr);

    Page *GetPages() { return pages_; }

    size_t GetPoolSize() { return pool_size_; }

protected:

    frame_id_t FindFrame();

    size_t pool_size_;
    Page *pages_;
    DiskManager *disk_manager_;
    LogManager *log_manager_;
    std::unordered_map<page_id_t, frame_id_t> page_table_;
    Replacer *replacer_;
    std::list<frame_id_t> free_list_;
    std::mutex latch_;
};

#endif //TICKETSYSTEM_BPLUSTREE_H

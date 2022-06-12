# Ticket System（火车票管理系统）

## 功能要求

- 对用户提供购票、车票查询、订票等操作
- 对管理员提供后台管理
- 本地存储用户数据、购票数据、车次数据

## 组员分工

- Limer：存储结构 B+树的实现
- 陈文韬：后端的主体逻辑

## 实现端口

### 文件读写：`file_io,h`

具体实现，之前已经在 Bookstore 中实现的 `MemoryRiver` 类，直接使用

### 文件存储：`b_plus_tree.h`

```cpp
// the license is included in the source file
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
  template <typename N>
  N *NewNode(page_id_t parent_id);

  void StartNewTree(const KeyType &key, const ValueType &value);

  bool InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction = nullptr);

  void InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                        Transaction *transaction = nullptr);

  Page *ReachLeafPage(const KeyType &key);

  template <typename N>
  N *Split(N *node);

  template <typename N>
  bool CoalesceOrRedistribute(N *node, Transaction *transaction = nullptr);

  template <typename N>
  bool Coalesce(N **neighbor_node, N **node, BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent,
                int index, Transaction *transaction = nullptr);

  template <typename N>
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
```

### 文件存储：`b_plus_tree_internal_page.h`

```cpp
// the license is included in the source files
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
```

### 文件存储：`b_plus_tree_leaf_page.h`

```cpp
// the license is included in the source files
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
```

### 缓存池：`buffer_pool_manager.h`

```cpp
// the license is included in the source files
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

```

### 指令读取：`Command.h`

```cpp
class Command {
    friend class AccountManagement;
    friend class TrainManagement;
private:
    string buffer = "";//存储字符串的缓冲区
    int cur = 0;//当前指针的位置
    char delimiter = ' ';//分隔符
    int timestamp; //时间戳

public:
    int cnt = 0;
    Command() = default; //构造函数
    Command(const Command &rhs);
    Command(char _delimiter);
    Command(const std::string &in, char _delimiter = ' ');
    ~Command() = default;
    void count(); //统计参数个数
    string next_token();//取出下一个Token,更新cur
    void clear();
    friend istream &operator>>(istream &input, Command &obj);
    friend ostream &operator<<(ostream &os, const Command &command);
    void set_delimiter(char new_delimiter); //修改分隔符
};
```

### 用户操作：`Account.h`

//需要使用之前写过的 vector 类

```cpp
class User{
private:
    char user_name[20], password[30], mail_addr[30], name[20];
    int privilege; //优先等级
    vector<Tickets> ticket; //购买的车票

public:
    User() = default; //构造函数
    bool operator<(const User &rhs) const;
}

class AccountManagement{
private:
    vector<User> login_pool;//登录池
    //可以用二叉查找树实现(红黑树)，以加快查询速度
    MemoryRiver<User> user_data;//保存数据
    BpTree<User> name_to_pos; //索引

public:
    AccountManagement() = default;

    void add_user(Command &line); //增加用户
    void login(Command &line); //登录
    void logout(Command &line); //登出
    void query_profile(Command &line); //查询用户信息
    void modify_profile(Command &line); //修改用户信息
}
```

### 车次&购票操作：`TrainSystem.h`

注：计算时间时，可能需要用到之前写过的 `calendar.hpp` 类。

```cpp
const int maxn = 100010; //最大车站数

class Train{ //一列火车
private:
    char train_ID[20], stations[maxn][30]; //途径车站
    int station_num, total_seat_num; //途径的车站数、座位数
    TimeType start_time, arrving_times[maxn], leaving_times[maxn]; //每日出发时间（hh-mm）
    //把给出的两站间的行车时间、停靠时间，转换为每一站的到达和离开时间
    TimeType start_sale_date, end_sale_date; //卖票的日期区间（2个mm-dd）
    char type; //列车类型
    bool is_released; //车次是否发布，如果未发布就不能售票
    int price_sum[maxn]; //用前缀和快速查询区间的票价和

public:
    Train() = default;
    bool operator<(const User &rhs) const;
}

class Station{ //属于某个车次的车站，额外存储一次信息
private:
    char train_ID[20], station_name[30];
    TimeType start_sale_time, end_sale_time, arriving_time, leaving_time; //该车次中，到站与出站时间
    int price_sum; //继承自Train
}

class Ticket{ //车票
private:
    Station s, t; //起始站和终点站
public:
    Tickets() = default;
    inline int time() const; //查询总时间
    inline int cost() const; //查询总价格
}

class DayTrain{ //车次，用来维护座位信息
private:
    int seat_num[maxn]; //到每一站所剩的座位数

public:
    int query_seat(int l, int r); //查询[l,r]最多能坐的人数
    int modify_seat(int l, int r, int val); //区间修改
}

class TrainManagement{ //总接口
private:
    MemoryRiver<Train> train_data; //车次数据
    MemoryRiver<Tickets> ticket_data; //购票数据

    BpTree<Train> id_to_pos; //索引
    BpTree<Tickets> time_to_pos, cost_to_pos; //按照不同关键字排序
    BpTree<>


public:
    TrainManagement() = default;
    //返回值设计为string，可以传递报错信息
    string add_train(Command &line); //增加列车
    string release_train(Command &line); //发布列车，可售票
    string query_train(Command &line); //查询车次
    string query_ticket(Command &line); //查询车票
    string query_transfer(Command &line);
    string buy_ticket(Command &line, AccountManagement &accounts);
    string query_order(Command &line, AccountManagement &accounts);
    string refund_ticket(Command &line, AccountManagement &accounts);

    string rollback(Command &line, AccountManagement & accounts);
    string clean(AccountManagement &accounts);
    string exit(AccountManagement &accounts); //退出系统，所有用户下线
}
```

### 其他库：`Library.h`

```cpp
//之前写好的类
class vector{}
class RedBlackTree{}
class calendar{}

//时间类
class TimeType{
private:
    int minute; //表示当前的时间距离 起始时间2021-6-1 00:00 有多少 分钟

public:
    TimeType() = default;
    TimeType(string s); //通过：Month-Day Hour:Minute 字符串来构造
    string transfer(); //转化为形如：Month-Day Hour:Minute 的字符串
    int get_month();
    int get_day();
    int get_hour();
    int get_minute();
}
```

### 主程序实现：`main.cpp`

```cpp
int main() {
    Command cmd;

}
```

## Bonus

选择的是，备份，缓存，并发。

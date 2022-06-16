# Ticket System（火车票管理系统）

## 功能要求

- 对用户提供购票、车票查询、订票等操作
- 对管理员提供后台管理
- 本地存储用户数据、购票数据、车次数据

## 组员分工

- Limer：存储结构B+树的实现
- Leonard：后端的主体逻辑

## 项目架构

分为两部分：

- `database`：由 `Bpt` 和文件读写、`vector` 等组成，由 Limer 完成
- `backend`：火车票系统的后端逻辑，包括命令读入、账户管理、车票管理，由 Leonrad 完成

Tips：把所有的类都放进 `namespace thomas` 中，便于管理。

### B+树：

1. `b_plus_tree.h`：B+树的主体部分

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

2. `b_plus_tree_internal_page.h`

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

3. `b_plus_tree_leaf_page.h`

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

4. `buffer_pool_manager.h`：缓存池

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

### 后端：

1.  `Command.h`：命令读取

```cpp
class Command {
    friend class AccountManagement;
    friend class TrainManagement;

private:
    string buffer = "";//存储字符串的缓冲区
    int cur = 0;//当前指针的位置
    char delimiter = ' ';//分隔符

public:
    int cnt = 0, timestamp = 0;//时间戳

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

2. `Account.h`：用户信息的存储

```cpp
class User {
        friend class AccountManagement;

    private:
        char user_name[22], password[32], mail_addr[32],
                name[32];  //数组大小要略大于实际长度，而且不能大太多。否则disk_limit
        int privilege; //优先等级
        //    vector<Tickets> ticket; //购买的车票

    public:
        User() = default; //构造函数
        User(const string &_username, const string &_name, const string &_mail,
             const string &_password, const int &_p);

        bool operator<(const User &rhs) const;
        string get_id();
    };
```


3. `TrainSystem.h`：列车本体、车次、车票、车站的存储

```cpp
class TrainManagement;

    class Train { //一列火车
        friend class TrainManagement;

    private:
        char train_ID[22], stations[maxn][32]; //途径车站
        int station_num, total_seat_num;       //途径的车站数、座位数
        TimeType start_time, arriving_times[maxn],
                leaving_times[maxn]; //每日出发时间（hh-mm）
        //把给出的两站间的行车时间、停靠时间，转换为每一站的到达和离开时间
        //下标从1开始存，而且都是相对于 start_time 计算得到的
        TimeType start_sale_date, end_sale_date; //卖票的日期区间（2个mm-dd）
        char type;                               //列车类型
        bool is_released = false; //车次是否发布，如果未发布就不能售票
        int price_sum[maxn];      //用前缀和快速查询区间的票价和

    public:
        Train() = default;

        Train(const string &_train_ID, const int &_station_num,
              const int &_total_seat_num, const string &_stations,
              const string &_prices, const string &_start_time,
              const string &_travel_time, const string &_stop_over_times,
              const string &_sale_date, const string &_type);

        bool operator<(const Train &rhs) const; //不需要？
        string get_id();
        friend void OUTPUT(TrainManagement &all, const string &train_ID);
    };

    class Ticket;

    class Station { //属于某个车次的车站，额外存储一次信息;
        //同时可以加快查询
        friend class TrainManagement;

        friend class Ticket;

    private:
        char train_ID[22], station_name[32];
        TimeType start_sale_time, end_sale_time, arriving_time,
                leaving_time;     //该车次中，到站与出站时间
        int price_sum, index; //继承自Train，index表示是该车次的第几站
        // todo : 同理把 station_name 和 train_id 复合起来,
        //  排序时，只需要考虑station_name

    public:
        Station() = default;

        Station(const string &_train_ID, const string &_station_name,
                const int &_price_sum, const TimeType &_start_sale_time,
                const TimeType &_end_sale_time, const TimeType &_arriving_time,
                const TimeType &_leaving_time, const int &_index);

        string get_id();

        friend bool time_cmp(const Ticket &a, const Ticket &b);

        friend bool cost_cmp(const Ticket &a, const Ticket &b);
    };

    class Ticket { //一张车票
        friend class TrainManagement;

    private:
        Station s, t; //起始站和终点站
        //因为 Station 是某一车次下的 车站，所以不用存储额外信息
    public:
        Ticket() = default;

        Ticket(const Station &x, const Station &y); //起点为x,终点为y
        int time() const;                           //查询总时间
        int cost() const;                           //查询总价格

        friend bool time_cmp(const Ticket &a, const Ticket &b);

        friend bool cost_cmp(const Ticket &a, const Ticket &b);
    };

    class DayTrain { //每天的车次，用来维护座位信息
        friend class TrainManagement;

    private:
        int seat_num[maxn]; //到每一站所剩的座位数
        TimeType start_day;
        char train_ID[22];
        // todo : 直接把 train_id 和 start_day 拼接起来,
        //  train_id是第一关键字
        //注意是 date，没有时间，如果不是要先 get_date
        // start_day指的是 每趟车从起点站发车的日期

    public:
        DayTrain() = default;

        int query_seat(int l, int r); //查询第l站到第r站,最多能坐的人数
        void modify_seat(int l, int r, int val); //区间修改
        string get_id();

        friend void OUTPUT(TrainManagement &all, const string &train_ID);
    };

    enum Status {
        success, pending, refunded
    };

    class Order { //订单
        friend class TrainManagement;

    private:
        char user_name[22], train_ID[22]; //用户名，车次
        int num, price, order_ID; //订单编号,从1开始(充当下单时间，用来排序)
        // num是票数, price 是单价
        // todo: 实际上，ull中的关键字是 user_name + order_ID，应该修改为pair类型
        //  排序时，以 order_id 为关键字，
        //  但是存储时是以user_name为第一关键字

        TimeType start_day, leaving_time, arriving_time;
        Status status;                         //订单当前状态
        int from, to;                          //起点和终点的 index
        char from_station[32], to_station[32]; //起点和终点

    public:
        Order() = default;

        Order(const string &_user_name, const string &_train_ID, const int &_num,
              const int &_price, const int &_order_ID, const TimeType &_start_day,
              const TimeType &_leaving_time, const TimeType &_arriving_time,
              const Status &_status, const int &_from, const int &_to,
              const string &_from_station, const string &_to_station);
        string get_id();

        friend bool order_cmp(const Order &a, const Order &b);
    };

    class PendingOrder { //候补的订单
        friend class TrainManagement;

    private:
        char train_ID[22], user_name[22];
        TimeType start_day;
        int num, from, to, order_ID;
        // todo: 关键字是 train_ID + start_sale_date + order_ID
        //  把 start_sale_date 和 order_ID 合成为一个 pair,然后与 train_ID 复合
        //  排序时，以 order_id 为关键字
        //  同理，但是存储时是以train_ID为第一关键字
        int pos; // refund_ticket 时临时用来存储位置

    public:
        PendingOrder() = default;

        PendingOrder(const string &_train_ID, const string &_user_name,
                     const TimeType &_start_day, const int &_num, const int &_from,
                     const int &_to, const int &_order_ID);
        string get_id();

        friend bool pending_order_cmp(const PendingOrder &a, const PendingOrder &b);
    };
```

4. `Management.h`

- 把 Account 和 TicketSystem 合并起来，作为 Bpt 的接口
- 作为命令执行的部分，同时内置了`rollback` 指令需要的 `Record_stack` 类

```cpp
template<typename T>
    class Record_stack {
        //        template<typename T>
        class Record { //内置
        public:
            int type, time; // type=0:insert, 1:delete, 2:modify
            T data;

            Record() = default;

            Record(const int &_type, const int &_time, const T &_data)
                    : data(_data), type(_type), time(_time) {}
        };

    private:
        StackManager<Record> stack;

    public:
        Record_stack() = default;

        Record_stack(const string &file_name);

        void add(const int &_type, const int &_time, const T &_data);

        Record pop(); //后退到t时刻

        bool empty();
    };

    class AccountManagement {
        friend class TrainManagement;

    private:
        sjtu::map<string, int> login_pool; //登录池,username -> privilege
        //可以用二叉查找树实现(红黑树)，以加快查询速度
        //之前大作业写好的 map 类
        //更新：为了保证实时修改，在权限发生修改的时候，要同时修改 map 中的值
        
        BPlusTreeIndexNTS<String<24>, User, StringComparator<24>> *user_database;
        StringComparator<24> cmp1;

        Record_stack<User> user_stack; //用于 rollback

    public:
        AccountManagement();

        //AccountManagement(const string &file_name);

        ~AccountManagement();

        string add_user(Command &line);       //增加用户
        string login(Command &line);          //登录
        string logout(Command &line);         //登出
        string query_profile(Command &line);  //查询用户信息
        string modify_profile(Command &line); //修改用户信息
    };

    class TrainManagement {
    private:
        //其类型在默认构造函数中指定，不能在这里写
        // Bpt中元素的排序规则
        StringComparator<24> cmp1;
        DualStringComparator<32, 24> cmp2;
        StringAnyComparator<24, int> cmp3;
        StringAnyComparator<24, int> cmp4;
        StringIntIntComparator<24> cmp5; //

        BPlusTreeIndexNTS<String<24>, Train, StringComparator<24>> *train_database;
        BPlusTreeIndexNTS<DualString<32, 24>, Station, DualStringComparator<32, 24>>
                *station_database;
        BPlusTreeIndexNTS<StringAny<24, int>, DayTrain, StringAnyComparator<24, int>>
                *daytrain_database;
        BPlusTreeIndexNTS<StringAny<24, int>, Order, StringAnyComparator<24, int>>
                *order_database;
        BPlusTreeIndexNTS<StringIntInt<24>, PendingOrder, StringIntIntComparator<24>>
                *pending_order_database;

        int order_num; //临时存储 order 总数

        //用于回滚的stack
        Record_stack<Train> train_stack;
        Record_stack<Station> station_stack;
        Record_stack<DayTrain> daytrain_stack;
        Record_stack<Order> order_stack;
        Record_stack<PendingOrder> pending_order_stack;

    public:
        friend void OUTPUT(TrainManagement &all, const string &train_ID);

        TrainManagement();

        //TrainManagement(const string &file_name);
        ~TrainManagement();

        //返回值设计为string，可以传递报错信息
        string add_train(Command &line);      //增加列车
        string release_train(Command &line);  //发布列车，可售票
        string query_train(Command &line);    //查询车次
        string delete_train(Command &line);   //删除列车
        string query_ticket(Command &line);   //查询车票
        string query_transfer(Command &line); //查询换乘
        string buy_ticket(Command &line, AccountManagement &accounts); //买票

        string query_order(Command &line, AccountManagement &accounts); //查询订单
        string refund_ticket(Command &line, AccountManagement &accounts); //退款
        string rollback(Command &line, AccountManagement &accounts); //回滚数据
        string clean(AccountManagement &accounts); //清空数据
        string exit(AccountManagement &accounts); //退出系统，所有用户下线
    };
```

5. `Library.h` ：包含了时间管理类

```cpp
extern int string_to_int(const string &s); //防止 multiple definition

static int Month[3] = {30, 31, 31}; //只在这个文件里有用

//注意：对于纯日期，要 + " 00:00"，对于纯时刻，要在前面加上 "06-01 "
class TimeType{
private:
    int minute; //表示当前的时间距离 起始时间2021-6-1 00:00 有多少 分钟

public:
    TimeType() : minute(0) {};

    TimeType(const int &x) : minute(x) {}

    TimeType(const string &s) { //通过：Month-Day Hour:Minute 字符串来构造
    }

    string transfer() const { //转化为形如：Month-Day Hour:Minute 的字符串
    }
    
    TimeType operator+(const TimeType &rhs) const {}

    TimeType operator+(const int &x) const {}

    TimeType operator+=(const int &x) { //要修改本身，不能加 const
    }

    TimeType operator-=(const int &x) {}

    int operator-(const TimeType &rhs) const{}

    bool operator==(const TimeType &rhs) const {}

    bool operator!=(const TimeType &rhs) const {}

    bool operator<(const TimeType &rhs) const {}

    bool operator>(const TimeType &rhs) const {}

    bool operator<=(const TimeType &rhs) const {}

    bool operator>=(const TimeType &rhs) const {}

    friend ostream &operator<<(ostream &os, const TimeType &type) {}

    TimeType get_date() const{ //返回当前日期 xx-xx
    }

    TimeType get_time() const{//返回当前时间 xx:xx
    }

    int get_value() const{} //返回存储的 minute 值
};
```

### 主程序实现：`main.cpp`

```cpp
#include "Account.h"
#include "Command.h"
#include "Management.h"
#include "TrainSystem.h"

using namespace std;
using namespace thomas;

AccountManagement accounts; //声明在外部，防止数组太大，爆栈空间
TrainManagement trains;

int main() {
    string input;

    while (getline(cin, input)) {
        Command cmd(input);
        string time = cmd.next_token();
        int l = time.length();
        cmd.timestamp = string_to_int(time.substr(1, l - 2));

        printf("[%d] ", cmd.timestamp);
        string s = cmd.next_token();
        if (s == "add_user")
            printf("%s\n", accounts.add_user(cmd).c_str());
        else if (s == "login")
            printf("%s\n", accounts.login(cmd).c_str());
        else if (s == "logout")
            printf("%s\n", accounts.logout(cmd).c_str());
        else if (s == "query_profile")
            printf("%s\n", accounts.query_profile(cmd).c_str());
        else if (s == "modify_profile")
            printf("%s\n", accounts.modify_profile(cmd).c_str());

        else if (s == "add_train")
            printf("%s\n", trains.add_train(cmd).c_str());
        else if (s == "release_train")
            printf("%s\n", trains.release_train(cmd).c_str());
        else if (s == "query_train")
            printf("%s\n", trains.query_train(cmd).c_str());
        else if (s == "delete_train")
            printf("%s\n", trains.delete_train(cmd).c_str());
        else if (s == "query_ticket")
            printf("%s\n", trains.query_ticket(cmd).c_str());
        else if (s == "query_transfer")
            printf("%s\n", trains.query_transfer(cmd).c_str());

        else if (s == "buy_ticket")
            printf("%s\n", trains.buy_ticket(cmd, accounts).c_str());
        else if (s == "query_order")
            printf("%s\n", trains.query_order(cmd, accounts).c_str());
        else if (s == "refund_ticket")
            printf("%s\n", trains.refund_ticket(cmd, accounts).c_str());
        else if (s == "rollback")
            printf("%s\n", trains.rollback(cmd, accounts).c_str());
        else if (s == "clean")
            printf("%s\n", trains.clean(accounts).c_str());
        else if (s == "exit")
            trains.exit(accounts);
    }

    return 0;
}
```

## Bonus选择

- Limer：缓存，并发
- Leonard：备份，命令行交互
   
   - 更准确地说，是定时自动备份，以及数据的导入导出。

## 备份功能说明

命令：

1. `export`
   
   - 参数：`-n`
   - 描述：将当前的数据导出到 `backup/-n(num)` 中
   - 返回：备份成功，

2. `import`

    - 参数：`-n`
    - 描述：将 `backup/-n(num)` 中的数导入到系统中

3. `backup`

    - 描述：将当前的数据备份为一个版本，也存储在 `backup` 中
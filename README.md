# Ticket System（火车票管理系统）

## 功能要求

- 对用户提供购票、车票查询、订票等操作
- 对管理员提供后台管理
- 本地存储用户数据、购票数据、车次数据

## 组员分工

- 吴林昱：存储结构B+树的实现
- 陈文韬：后端命令，用户、车票

## 实现端口

### 文件读写：`file_io,h`

具体实现，之前已经在Bookstore中实现的 `MemoryRiver` 类，直接使用

### 文件存储：`BpTree.h`

//注意，此处作废，实现时仅以Limerancy的为准

```cpp
const int M = 4; //初始数组的大小

template<class Key, class T, class Compare = std::less<Key>>
class BpTree{
    //----------------------成员变量-------------------------
private:    
    class BpNode{
        int key_num; //当前节点关键字的数目
        Key key[M + 1]; //记录关键字
        BpNode *parent; //指向父节点的指针
        BpNode *son[M + 1]; //指向子节点的指针
        BpNode* nxt; //指向单链表的下一个节点
        T *record[M + 1]; //指向记录的指针
        
        //默认构造
        BpNode() : key_num(0), parent(nullptr){
            for (int i = 0;i <= M; ++i) {
                son[i] = nullptr;
                record[i] = nullptr;
            }
        }
        
        //复制构造
        BpNode(const BpNode &other) {}
       	
        //析构函数
        ~BpNode() {
            key_num = 0;
            for (int i = 0;i <= M; ++i) {
                if (son[i]) delete son[i];
            }
            if (parent) delete parent;
        }        
    }
    
    
    BpNode *root, *head, *rear; //head和rear用来存储单链表
    MemoryRiver<> Tree;
    
    //--------------------成员函数--------------------------
public:
    BpTree();
    BpTree(const BpTree &other);
    ~BpTree();
    
    void init(); //初始化
    BpNode* malloc_new_node(); //生成新的BpNode，并为其分配空间
    BpNode* insert(const Key &_key, BpNode* p); //插入
    BpNode* remove(const Key &_key, BpNode* p); //删除
    BpNode* clear(BpNode* p);  //清空p和他的子树
    void travel(BpNode* p); //遍历p的子节点的所有数据
    BpNode* find(const Key &_key); //查找键值为_key的节点
    BpNode* split(BpNode* X);//分裂过大的节点X
    BpNode* merge(BpNode* X, BpNode* S);//合并过小的节点X和S
    BpNode* find_sibling(BpNode* X);//查找兄弟节点
}
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

//需要使用之前写过的vector类

```cpp
class User{
private:    
    char user_name[20], password[30], mail_addr[30], name[20]; 
    int priviledge; //优先等级
    vector<Tickets> ticket; //购买的车票
    
public:
    User() = default; //构造函数
    bool operator<(const User &rhs) const;
}

class AccountManagement{
private:
    vector<User> login_pool;//登录池，需要查找
    MemoryRiver<User> user_data;//保存数据
    BpTree id_to_pos; //索引

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
class Train{ //车次
private:    
    char train_ID[20], stations[100010][30]; //途径车站
    int station_num, seat_num, prices[100010]; //途径的车站数、座位数、两站间的票价
    int start_time[2], travel_times[100010], stop_over_times[100010]; //每日出发时间（hh-mm）、两站间的行车时间、停靠时间
    int sale_date[2][2]; //卖票的日期区间（2个mm-dd）
    char type; //列车类型
    bool is_released; //车次是否发布，如果未发布就不能售票
    int sum_price[100010]; //用前缀和快速查询区间的票价和

public:
    Train() = default;
    bool operator<(const User &rhs) const;
}

class Tickets{ //车票（一堆）
private:
    int date[2]; //出发日期（mm-dd）
    char start_station[30], arrival_station[30]; //起点站和终点站
    int num, price; 
    
}

class TrainManagement{ //总接口
private:
    MemoryRiver<Train> train_data; //车次数据
    MemoryRiver<Tickets> ticket_data; //购票数据
    BpTree<Train> id_to_pos; //索引
    BpTree<Tickets> time_to_pos, cost_to_pos; //按照不同关键字排序

public:
    TrainManagement() = default;
    
    void add_train(Command &line); //增加列车
    void release_train(Command &line); //发布列车，可售票
    void query_train(Command &line); //查询车次
    void query_ticket(Command &line); //查询车票
    void query_transfer(Command &line); 
    void buy_ticket(Command &line, AccountManagement &accounts);
    void query_order(Command &line, AccountManagement &accounts);
    void refund_ticket(Command &line, AccountManagement &accounts);

    void rollback(Command &line, AccountManagement & accounts);
    void clean(AccountManagement &accounts);
    void exit(AccountManagement &accounts); //退出系统，所有用户下线
}
```

### 订单记录：`Order.h`

```cpp
```



### ~~记录时间戳：`log.h`~~ 

//直接在command中实现

### 主程序实现：`main.cpp`

```cpp
int main() {
    
}
```


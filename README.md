# TicketSystem

# 火车票管理系统

## 功能要求

- 对用户提供购票、车票查询、订票等操作
- 对管理员提供后台管理
- 本地存储用户数据、购票数据、车次数据

## 实现端口

### 文件读写：`file_io,h`

具体实现，之前已经在Bookstore中实现的 `MemoryRiver` 类，直接使用

### 文件存储：`BpTree.h`

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

### 指令读取

 


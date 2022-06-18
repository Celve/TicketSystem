//
// Created by Leonard C on 2022/6/9.
//

#ifndef TICKETSYSTEM_MANAGEMENT_H
#define TICKETSYSTEM_MANAGEMENT_H

#include <future>

#include "Account.h"
#include "TrainSystem.h"
#include "common/config.h"
#include "storage/index/b_plus_tree_index_nts.h"
#include "thread/lock_pool.h"
#include "thread/thread_pool.h"
#include "type/string_any.h"
#include "type/string_int_int.h"

namespace thomas {

#define RETURN_TYPE std::future<std::string>

class AccountManagement {
  friend class TrainManagement;

private:
  sjtu::map<string, int> login_pool; //登录池,username -> privilege
  //可以用二叉查找树实现(红黑树)，以加快查询速度
  //更新：不能保证实时修改，所以login_pool里面的权限没有用...

  //    MemoryRiver<User> user_data;//保存数据
  //    Ull username_to_pos; //索引，暂时用 Ull 完成，最后要改为 BpTree

  // Memory_river与 ull 的复合
  BPlusTreeIndexNTS<String<24>, User, StringComparator<24>> *user_database;
  LockPool lock_pool_[MUTEX_NUMBER];
  ThreadPool *thread_pool_;
  StringComparator<24> cmp1;

public:
  AccountManagement(ThreadPool *thread_pool);
  AccountManagement(const string &file_name);
  ~AccountManagement();

  RETURN_TYPE add_user(Command &line);       //增加用户
  RETURN_TYPE login(Command &line);          //登录
  RETURN_TYPE logout(Command &line);         //登出
  RETURN_TYPE query_profile(Command &line);  //查询用户信息
  RETURN_TYPE modify_profile(Command &line); //修改用户信息
};

class TrainManagement {
private:
  //        MemoryRiver<Train> train_data; //车次数据
  //        MemoryRiver<DayTrain> day_train_data; //每日座位数据
  //        MemoryRiver<Station> station_data; //车站数据
  //        MemoryRiver<Order> order_data; //订单数据
  //        MemoryRiver<PendingOrder> pending_order_data;
  //    Ull train_id_to_pos, daytrain_id_to_pos, station_id_to_pos; //索引
  //    Ull order_id_to_pos, pending_order_id_to_pos;

  //其类型在默认构造函数中指定，不能在这里写？
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

  //临时数组的大小不是110
  int order_num; //临时存储 order 总数

  LockPool lock_pool_[MUTEX_NUMBER];
  ThreadPool *thread_pool_;

public:
  friend void OUTPUT(TrainManagement &all, const string &train_ID);

  TrainManagement(ThreadPool *thread_pool);
  //    TrainManagement(const string &file_name);
  ~TrainManagement();

  //返回值设计为string，可以传递报错信息
  RETURN_TYPE add_train(Command &line);      //增加列车
  RETURN_TYPE release_train(Command &line);  //发布列车，可售票
  RETURN_TYPE query_train(Command &line);    //查询车次
  RETURN_TYPE delete_train(Command &line);   //删除列车
  RETURN_TYPE query_ticket(Command &line);   //查询车票
  RETURN_TYPE query_transfer(Command &line); //查询换乘
  RETURN_TYPE buy_ticket(Command &line, AccountManagement &accounts);
  RETURN_TYPE query_order(Command &line, AccountManagement &accounts);
  RETURN_TYPE refund_ticket(Command &line, AccountManagement &accounts);
  RETURN_TYPE rollback(Command &line, AccountManagement &accounts);
  RETURN_TYPE clean(AccountManagement &accounts);
  RETURN_TYPE exit(AccountManagement &accounts); //退出系统，所有用户下线
};

} // namespace thomas

#endif // TICKETSYSTEM_MANAGEMENT_H

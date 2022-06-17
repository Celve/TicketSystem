//
// Created by Leonard C on 2022/6/9.
//

#ifndef TICKETSYSTEM_MANAGEMENT_H
#define TICKETSYSTEM_MANAGEMENT_H

#include "Account.h"
#include "TrainSystem.h"
#include "storage/index/b_plus_tree_index_nts.h"
#include "storage/index/b_plus_tree_index_pool.h"
#include "type/string_any.h"
#include "type/string_int_int.h"

namespace thomas {
class AccountManagement {
  friend class TrainManagement;

private:
  sjtu::map<string, int> login_pool; //登录池,username -> privilege
  //可以用二叉查找树实现(红黑树)，以加快查询速度
  //更新：不能保证实时修改，所以login_pool里面的权限没有用...

  //    MemoryRiver<User> user_data;//保存数据
  //    Ull username_to_pos; //索引，暂时用 Ull 完成，最后要改为 BpTree

  // Memory_river与 ull 的复合
  BPlusTreeIndexPool<String<24>, User, StringComparator<24>> *user_database;
  StringComparator<24> cmp1;

public:
  AccountManagement();
  AccountManagement(const string &file_name);
  ~AccountManagement();

  string add_user(Command &line);       //增加用户
  string login(Command &line);          //登录
  string logout(Command &line);         //登出
  string query_profile(Command &line);  //查询用户信息
  string modify_profile(Command &line); //修改用户信息
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

  BPlusTreeIndexPool<String<24>, Train, StringComparator<24>> *train_database;
  BPlusTreeIndexPool<DualString<32, 24>, Station, DualStringComparator<32, 24>>
      *station_database;
  BPlusTreeIndexPool<StringAny<24, int>, DayTrain, StringAnyComparator<24, int>>
      *daytrain_database;
  BPlusTreeIndexPool<StringAny<24, int>, Order, StringAnyComparator<24, int>>
      *order_database;
  BPlusTreeIndexPool<StringIntInt<24>, PendingOrder, StringIntIntComparator<24>>
      *pending_order_database;

  //临时数组的大小不是110
  int order_num; //临时存储 order 总数

public:
  friend void OUTPUT(TrainManagement &all, const string &train_ID);

  TrainManagement();
  //    TrainManagement(const string &file_name);
  ~TrainManagement();

  //返回值设计为string，可以传递报错信息
  string add_train(Command &line);      //增加列车
  string release_train(Command &line);  //发布列车，可售票
  string query_train(Command &line);    //查询车次
  string delete_train(Command &line);   //删除列车
  string query_ticket(Command &line);   //查询车票
  string query_transfer(Command &line); //查询换乘
  string buy_ticket(Command &line, AccountManagement &accounts);
  string query_order(Command &line, AccountManagement &accounts);
  string refund_ticket(Command &line, AccountManagement &accounts);
  string rollback(Command &line, AccountManagement &accounts);
  string clean(AccountManagement &accounts);
  string exit(AccountManagement &accounts); //退出系统，所有用户下线
};

} // namespace thomas

#endif // TICKETSYSTEM_MANAGEMENT_H

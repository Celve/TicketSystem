//
// Created by Leonard C on 2022/6/9.
//

#ifndef TICKETSYSTEM_MANAGEMENT_H
#define TICKETSYSTEM_MANAGEMENT_H

#include "Account.h"
#include "TrainSystem.h"
#include "storage/index/b_plus_tree_index_nts.h"
#include "type/string_any.h"
#include "type/string_int_int.h"

namespace thomas {
    class AccountManagement {
        friend class TrainManagement;

    private:
        sjtu::map<string, int> login_pool;//登录池,username -> privilege
        //可以用二叉查找树实现(红黑树)，以加快查询速度
        //更新：不能保证实时修改，所以login_pool里面的权限没有用...

//    MemoryRiver<User> user_data;//保存数据
//    Ull username_to_pos; //索引，暂时用 Ull 完成，最后要改为 BpTree

        //Memory_river与 ull 的复合
        BPlusTreeIndexNTS<String<32>, User, StringComparator<32> > *user_database;
        StringComparator<32> cmp1;

    public:
        AccountManagement();
        AccountManagement(const string &file_name);
        ~AccountManagement();

        string add_user(Command &line); //增加用户
        string login(Command &line); //登录
        string logout(Command &line); //登出
        string query_profile(Command &line); //查询用户信息
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
        //Bpt中元素的排序规则
        StringComparator<32> cmp1;
        DualStringComparator<32, 32> cmp2;
        StringAnyComparator<32, int> cmp3;
        StringAnyComparator<32, int> cmp4;
        StringIntIntComparator<32> cmp5; //

        BPlusTreeIndexNTS<String<32>, Train, StringComparator<32> > *train_database;
        BPlusTreeIndexNTS<DualString<32, 32>, Station, DualStringComparator<32, 32> > *station_database;
        BPlusTreeIndexNTS<StringAny<32, int>, DayTrain, StringAnyComparator<32, int> > *daytrain_database;
        BPlusTreeIndexNTS<StringAny<32, int>, Order, StringAnyComparator<32, int> > *order_database;
        BPlusTreeIndexNTS<StringIntInt<32>, PendingOrder, StringIntIntComparator<32> > *pending_order_database;

        //临时数组的大小不是110
        int order_num; //临时存储 order 总数

    public:
        friend void OUTPUT(TrainManagement &all, const string &train_ID);

        TrainManagement();
//    TrainManagement(const string &file_name);
        ~TrainManagement();

        //返回值设计为string，可以传递报错信息
        string add_train(Command &line); //增加列车
        string release_train(Command &line); //发布列车，可售票
        string query_train(Command &line); //查询车次
        string delete_train(Command &line); //删除列车
        string query_ticket(Command &line); //查询车票
        string query_transfer(Command &line); //查询换乘
        string buy_ticket(Command &line, AccountManagement &accounts);
        string query_order(Command &line, AccountManagement &accounts);
        string refund_ticket(Command &line, AccountManagement &accounts);
        string rollback(Command &line, AccountManagement &accounts);
        string clean(AccountManagement &accounts);
        string exit(AccountManagement &accounts); //退出系统，所有用户下线
    };

}


#endif //TICKETSYSTEM_MANAGEMENT_H

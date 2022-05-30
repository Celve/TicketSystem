#ifndef TicketYSTEM_TRAINSYSTEM_H
#define TicketYSTEM_TRAINSYSTEM_H

#include "Account.h"
//#include "Library.h"

//extern bool time_cmp();
//extern bool cost_cmp();

const int maxn = 100010; //最大车站数
const int MAX_INT = 0x7fffffff;

class TrainID{
    friend class Train;
    friend class TrainManagement;

private:
    char s;
};

class Train{ //一列火车
    friend class TrainManagement;
private:
    char train_ID[25], stations[maxn][32]; //途径车站
    int station_num, total_seat_num; //途径的车站数、座位数
    TimeType start_time, arriving_times[maxn], leaving_times[maxn]; //每日出发时间（hh-mm）
    //把给出的两站间的行车时间、停靠时间，转换为每一站的到达和离开时间
    //下标从1开始存，而且都是相对于 start_time 计算得到的
    TimeType start_sale_date, end_sale_date; //卖票的日期区间（2个mm-dd）
    char type; //列车类型
    bool is_released = false; //车次是否发布，如果未发布就不能售票
    int price_sum[maxn]; //用前缀和快速查询区间的票价和

public:
    Train() = default;
    Train(const string &_train_ID, const int &_station_num, const int &_total_seat_num,
          const string &_stations, const string &_prices, const string & _start_time,
          const string &_travel_time, const string &_stop_over_times, const string &_sale_date,
          const string &_type);
    bool operator<(const Train &rhs) const; //不需要？
};

class Ticket;
class Station{ //属于某个车次的车站，额外存储一次信息;
    //同时可以加快查询
    friend class TrainManagement;
    friend class Ticket;
private:
    char train_ID[25], station_name[32];
    TimeType start_sale_time, end_sale_time, arriving_time, leaving_time; //该车次中，到站与出站时间
    int price_sum, index; //继承自Train，index表示是该车次的第几站
    char id[64]; //这里只起到额外存储和提示作用
    //todo : 同理直接把 train_id 和 station_name 拼接起来

public:
    Station() = default;
    Station(const string &_train_ID, const string &_station_name, const int &_price_sum,
            const TimeType &_start_sale_time, const TimeType &_end_sale_time,
            const TimeType &_arriving_time, const TimeType &_leaving_time, const int &_index);
    friend bool time_cmp(const Ticket &a, const Ticket &b);
    friend bool cost_cmp(const Ticket &a, const Ticket &b);
};

class Ticket{ //一张车票
    friend class TrainManagement;
private:
    Station s, t; //起始站和终点站
    //因为 Station 是某一车次下的 车站，所以不用存储额外信息
public:
    Ticket() = default;
    Ticket(const Station &x, const Station &y); //起点为x,终点为y
    inline int time() const; //查询总时间
    inline int cost() const; //查询总价格
    friend bool time_cmp(const Ticket &a, const Ticket &b);
    friend bool cost_cmp(const Ticket &a, const Ticket &b);
};

class DayTrain{ //每天的车次，用来维护座位信息
    friend class TrainManagement;
private:
    int seat_num[maxn]; //到每一站所剩的座位数
    char id[64];
    //todo : 直接把 train_id 和 start_day 拼接起来
    //注意是 date，没有时间，如果不是要先 get_date
    //start_day指的是 每趟车从起点站发车的日期

public:
    DayTrain() = default;
    int query_seat(int l, int r); //查询第l站到第r站,最多能坐的人数
    void modify_seat(int l, int r, int val); //区间修改
};

enum Status {success, pending, refunded};
class Order{ //订单
    friend class TrainManagement;
private:
    char user_name[25], train_ID[25]; //用户名，车次
    int num, price, order_ID; //订单编号,从1开始(充当下单时间，用来排序)
    //num是票数, price 是单价
    char id[64], pos;
    //todo: 实际上，ull中的关键字是 user_name + order_ID，应该修改为pair类型

    TimeType start_day, leaving_time, arriving_time;
    Status status; //订单当前状态
    int from, to; //起点和终点的 index
    char from_station[32], to_station[32]; //起点和终点

public:
    Order() = default;
    Order(const string &_user_name, const string &_train_ID, const int &_num, const int &_price, const int &_order_ID,
          const TimeType &_start_day, const TimeType &_leaving_time, const TimeType &_arriving_time,
          const Status & _status, const int &_from, const int &_to, const string &_from_station, const string &_to_station);
    friend bool order_cmp(const Order &a, const Order &b);
};

class PendingOrder{ //候补的订单
    friend class TrainManagement;
private:
    char train_ID[25], user_name[25];
    TimeType start_day;
    int num, from, to, order_ID;
    char id[64], pos;
    //todo: 关键字是 train_ID + start_sale_date + order_ID

public:
    PendingOrder() = default;
    PendingOrder(const string &_train_ID, const string &_user_name, const TimeType &_start_day,
                 const int &_num, const int &_from, const int &_to, const int &_order_ID);
    friend bool pending_order_cmp(const PendingOrder &a, const PendingOrder &b);
};

//------------------------------------------总接口----------------------------------------

class TrainManagement{
private:
    MemoryRiver<Train> train_data; //车次数据
    MemoryRiver<DayTrain> day_train_data; //每日座位数据
    MemoryRiver<Station> station_data; //车站数据
//    MemoryRiver<Ticket> ticket_data; //购票数据
    MemoryRiver<Order> order_data; //订单数据
    MemoryRiver<PendingOrder> pending_order_data;

    Ull train_id_to_pos, daytrain_id_to_pos, station_id_to_pos; //索引
    Ull order_id_to_pos, pending_order_id_to_pos;

//    Ull time_to_pos, cost_to_pos; //按照不同关键字排序

    Ticket tickets[maxn]; //临时存储 query_ticket 的结果
    Order orders[maxn]; //临时存储 query_order 结果
    PendingOrder pending_orders[maxn];
    pair<string, int> starts[maxn], ends[maxn]; //临时存储 query_transfer 2趟车次的 所有车站
    int order_num; //临时存储 order 总数

public:
    TrainManagement();
//    TrainManagement(const string &file_name);

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

#endif //TicketYSTEM_TRAINSYSTEM_H

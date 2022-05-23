#include "TrainSystem.h"

template<typename T>
void Sort(T *a, int l, int r, bool cmp(const T &u, const T &v)) {
    if (l >= r) return;
    int pl = l, pr = r;
    T mid_val = a[(l + r) / 2];
    while (pl <= pr) {
        while (cmp(a[pl], mid_val)) ++pl;
        while (cmp(mid_val, a[pr])) --pr;
        if (pl <= pr) swap(a[pl], a[pr]), ++pl, --pr;
    }
    if (l < pr) Sort(a, l, pr, cmp);
    if (pl < r) Sort(a, pl, r, cmp);
}

//-------------------------------------------------class Train

Train::Train(const string &_train_ID, const int &_station_num, const int &_total_seat_num, const string &_stations,
             const string &_prices, const string &_start_time, const string &_travel_time,
             const string &_stop_over_times, const string &_sale_date, const string &_type) {
    strcpy(train_ID, _train_ID.c_str());
    station_num = _station_num;
    total_seat_num = _total_seat_num;
    type = _type[0];

    Command c1(_prices, '|');
    string tp = c1.next_token();
    int k = 1;
    price_sum[0] = price_sum[1] = 0; //到第一站不用钱
    while (!tp.empty()) {
        k++;
        price_sum[k] = price_sum[k - 1] + string_to_int(tp);
        tp = c1.next_token();
    }

    Command c2(_stations, '|');
    tp = c2.next_token();
    k = 0;
    while (!tp.empty()) {
        strcpy(stations[++k], tp.c_str());
        tp = c2.next_token();
    }

    start_time = TimeType("06-01 " + _start_time);
    Command c3(_sale_date, '|');
    start_sale_date = TimeType(c3.next_token() + " 00:00");
    end_sale_date = TimeType(c3.next_token() + " 00:00");

    Command c4(_travel_time, '|'), c5(_stop_over_times, '|');
    arriving_times[1] = TimeType(0);
    leaving_times[1] = start_time;
    for (int i = 1; i <= station_num - 2; ++i) { //可以自动判断 只有2站 的情况
        arriving_times[i + 1] = leaving_times[i] + string_to_int(c4.next_token());
        leaving_times[i + 1] = arriving_times[i + 1] + string_to_int(c5.next_token());
    }
    arriving_times[station_num] = leaving_times[station_num - 1] + string_to_int(c4.next_token());
    leaving_times[station_num] = MAX_INT;

    is_released = false;
}

bool Train::operator<(const Train &rhs) const {
    return strcmp(train_ID, rhs.train_ID) < 0;
}

//--------------------------------------------------class DayTrain

void DayTrain::modify_seat(int l, int r, int val) {
    for (int i = l; i <= r; ++i) seat_num[i] += val;
}

int DayTrain::query_seat(int l, int r) {
    int ans = 100000;
    for (int i = l; i <= r; ++i) ans = min(ans, seat_num[i]);
    return ans;
}

//--------------------------------------------------class Station

Station::Station(const string &_train_ID, const string &_station_name, const int &_price_sum,
                 const TimeType &_start_sale_time, const TimeType &_end_sale_time, const TimeType &_arriving_time,
                 const TimeType &_leaving_time, const int &_index)
        : price_sum(_price_sum), start_sale_time(_start_sale_time), end_sale_time(_end_sale_time),
          arriving_time(_arriving_time), leaving_time(_leaving_time), index(_index) {
    strcpy(train_ID, _train_ID.c_str());
    strcpy(station_name, _station_name.c_str());
}

//---------------------------------------------------class Ticket

Ticket::Ticket(const Station &x, const Station &y) : s(x), t(y) {}

int Ticket::cost() const {
    return t.price_sum - s.price_sum;
}

int Ticket::time() const {
    return t.arriving_time - s.leaving_time;
}

//这是在外部的函数！不在 Ticket 类中
//记得 train_ID 是第二关键字
bool time_cmp(const Ticket &a, const Ticket &b) {
    if (a.time() == b.time()) return strcmp(a.s.train_ID, b.s.train_ID) < 0;
    return a.time() < b.time();
}

bool cost_cmp(const Ticket &a, const Ticket &b) {
    if (a.cost() == b.cost()) return strcmp(a.s.train_ID, b.s.train_ID) < 0;
    return a.cost() < b.cost();
}

bool order_cmp(const Order &a, const Order &b) { //todo: 按照新的规则
}

//-------------------------------------------------class Order

Order::Order(const string &_user_name, const string &_train_ID, const int &_num, const int &_price,
             const int &_order_ID, const TimeType &_start_day, const TimeType &_leaving_time,
             const TimeType &_arriving_time, const Status &_status, const int &_from, const int &_to,
             const string &_from_station, const string &_to_station)
             : num(_num), price(_price), order_ID(_order_ID), start_day(_start_day), leaving_time(_leaving_time),
               arriving_time(_arriving_time), status(_status), from(_from), to(_to) {
    strcpy(user_name, _user_name.c_str());
    strcpy(train_ID, _train_ID.c_str());
    strcpy(from_station, _from_station.c_str());
    strcpy(to_station, _to_station.c_str());
}

//---------------------------------------------------class PendingOrder

PendingOrder::PendingOrder(const string &_train_ID, const string &_user_name, const TimeType &_start_day,
                           const int &_num, const int &_from, const int &_to, const int _order_ID)
                           : start_day(_start_day), num(_num), from(_from), to(_from), order_ID(_order_ID) {
    strcpy(train_ID, _train_ID.c_str());
    strcpy(user_name, _user_name.c_str());
}

//-------------------------------------------------class TrainManagement

TrainManagement::TrainManagement() {
    train_data.initialise("train_data");
    day_train_data.initialise("day_train_data");
    station_data.initialise("station_data");
    order_data.initialise("order_data");
    pending_order_data.initialise("pending_order_data");

    train_id_to_pos.init("train_id_to_pos");
    daytrain_id_to_pos.init("daytrain_id_to_pos");
    station_id_to_pos.init("station_id_to_pos");
    order_id_to_pos.init("order_id_to_pos");
    pending_order_id_to_pos.init("pending_order_id_to_pos");
}

string TrainManagement::add_train(Command &line) {
    string opt = line.next_token(), train_id, stations, prices, type;
    string start_time, travel_times, stop_over_times, sale_date;
    int seat_num = 0, station_num = 0;

    while (!opt.empty()) {
        if (opt == "-i") train_id = line.next_token();
        else if (opt == "-n") station_num = string_to_int(line.next_token());
        else if (opt == "-m") seat_num = string_to_int(line.next_token());
        else if (opt == "-s") stations = line.next_token();
        else if (opt == "-p") prices = line.next_token();
        else if (opt == "-x") start_time = line.next_token();
        else if (opt == "-t") travel_times = line.next_token();
        else if (opt == "-o") stop_over_times = line.next_token();
        else if (opt == "-d") sale_date = line.next_token();
        else type = line.next_token();

        opt = line.next_token();
    }

    vector<int> ans;
    train_id_to_pos.find_node(train_id, ans);
    if (!ans.empty()) return "-1"; //train_ID 已存在，添加失败

    Train new_train(train_id, station_num, seat_num, stations, prices, start_time, travel_times,
                    stop_over_times, sale_date, type);
    int pos = train_data.write(new_train);
    train_id_to_pos.add_node(UllNode(train_id, pos));
    return "0";
}

string TrainManagement::release_train(Command &line) {
    line.next_token(); //过滤-i
    string t_id = line.next_token();

    vector<int> ans;
    train_id_to_pos.find_node(t_id, ans);
    if (ans.empty()) return "-1"; //车次不存在，失败

    Train target_train;
    train_data.read(target_train, ans[0]);
    if (target_train.is_released) return "-1"; //重复发布，失败
    target_train.is_released = true;
    train_data.update(target_train, ans[0]);

    //维护 每天的车次座位数
    for (auto i = target_train.start_sale_date; i <= target_train.end_sale_date; i += 1440) {
        DayTrain tp_daytrain;
        for (int j = 1; j <= target_train.station_num; ++j) tp_daytrain.seat_num[j] = target_train.total_seat_num;

        int pos = day_train_data.write(tp_daytrain);

        //todo: 要修改，可能用 pair<> + hash 实现
        string key = t_id + i.transfer();
        //目前直接用 train_id + time 替代
        strcpy(tp_daytrain.id, key.c_str());
        daytrain_id_to_pos.add_node(UllNode(key, pos));
    }

    //维护 沿途的每个车站
    for (int i = 1; i <= target_train.station_num; ++i) {
        Station tp_station(t_id, target_train.stations[i], target_train.price_sum[i],
                           target_train.start_sale_date, target_train.end_sale_date,
                           target_train.arriving_times[i], target_train.leaving_times[i], i);
        int pos = station_data.write(tp_station);

        //todo: 要修改，不能直接暴力连接
        string key = t_id + string(target_train.stations[i]);
        //同理，目前直接用 train_id + station_name 替代
        strcpy(tp_station.id, key.c_str());
        station_id_to_pos.add_node(UllNode(key, pos));
    }

    return "0";
}

string TrainManagement::query_train(Command &line) {
    string opt = line.next_token(), t_id, date, output;
    while (!opt.empty()) {
        if (opt == "-i") t_id = line.next_token();
        else date = line.next_token();

        opt = line.next_token();
    }

    vector<int> ans;
    TimeType day(date + " 00:00");
    train_id_to_pos.find_node(t_id, ans);
    if (ans.empty()) return "-1"; //没有车
    Train target_train;
    train_data.read(target_train, ans[0]);

    DayTrain current_daytrain;
    vector<int> ans2;
    daytrain_id_to_pos.find_node(string(t_id + day.transfer()), ans2);
    day_train_data.read(current_daytrain, ans2[0]);

    //第一行
    output = t_id + " " + target_train.type + "\n";
    //未发布，则所有票都没卖--->实际上不需要特判
    if (!target_train.is_released) {
        //第二行
        output += string(target_train.stations[1]) + " xx-xx xx:xx -> " + (day + target_train.start_time).transfer() +
                  " 0 " + to_string(target_train.total_seat_num) + "\n";
        for (int i = 2; i <= target_train.station_num - 1; ++i) {
            output +=
                    string(target_train.stations[i]) + " " + (day + target_train.arriving_times[i]).transfer() + " -> "
                    + (day + target_train.leaving_times[i]).transfer() + " " + to_string(target_train.price_sum[i])
                    + " " + to_string(target_train.total_seat_num) + "\n";
        }
        //最后一行
        output += string(target_train.stations[target_train.station_num]) + " " +
                  (day + target_train.arriving_times[target_train.station_num]).transfer() + " -> xx-xx xx:xx " +
                  to_string(target_train.price_sum[target_train.station_num]) + " x";
    } else {
        //从 current_daytrain 获取实时的座位数
        output += string(target_train.stations[1]) + " xx-xx xx:xx -> " + (day + target_train.start_time).transfer() +
                  " 0 " + to_string(current_daytrain.seat_num[1]) + "\n";
        for (int i = 2; i <= target_train.station_num - 1; ++i) {
            output +=
                    string(target_train.stations[i]) + " " + (day + target_train.arriving_times[i]).transfer() + " -> "
                    + (day + target_train.leaving_times[i]).transfer() + " " + to_string(target_train.price_sum[i])
                    + " " + to_string(current_daytrain.seat_num[i]) + "\n";
        }
        output += string(target_train.stations[target_train.station_num]) + " " +
                  (day + target_train.arriving_times[target_train.station_num]).transfer() + " -> xx-xx xx:xx " +
                  to_string(target_train.price_sum[target_train.station_num]) + " x";
    }
    return output;
}

string TrainManagement::delete_train(Command &line) {
    line.next_token();
    string t_id = line.next_token();
    vector<int> ans;
    train_id_to_pos.find_node(t_id, ans);
    if (ans.empty()) return "-1"; //不存在，不能删

    Train target_train;
    train_data.read(target_train, ans[0]);
    if (target_train.is_released) return "-1"; //已发布，不能删

    train_data.Delete(ans[0]);
    train_id_to_pos.delete_node(UllNode(t_id, ans[0]));
    return "0";
}

string TrainManagement::query_ticket(Command &line) {
    string opt = line.next_token(), s, t, date, type = "time"; //默认按时间排序
    while (!opt.empty()) {
        if (opt == "-s") s = line.next_token();
        else if (opt == "-t") t = line.next_token();
        else if (opt == "-d") date = line.next_token();
        else if (opt == "-p") type = line.next_token();

        opt = line.next_token();
    }

    if (s == t) return "0"; //起点等于终点，显然无票
    TimeType day(date + " 00:00");
    vector<int> all, ans1, ans2;

    //todo:区间查找，查找所有 站点为 s 和 t 的 station 车站
    //因为关键字是直接拼接的，所以不好查，直接暴力遍历
    train_id_to_pos.find_all(all); //全部读取出来，是按照 train_ID 升序排列的
    for (int i = 0; i < all.size(); ++i) {
        Train tp_train;
        train_data.read(tp_train, all[i]);
        string key1 = string(tp_train.train_ID) + s, key2 = string(tp_train.train_ID) + t;

        station_id_to_pos.find_node(key1, ans1);
        station_id_to_pos.find_node(key2, ans2);
    }
    //上述代码要修改，最后保证查询到的不同车次的车站要存在两个 vector 中

    if (ans1.empty() || ans2.empty()) return "0"; //无票
    int cnt = 0;
    Station s1, t1; //起点和终点

    for (int i1 = 0, i2 = 0; i1 < ans1.size() && i2 < ans2.size();) {
        station_data.read(s1, ans1[i1]), station_data.read(t1, ans2[i2]);
        //判断是否为同一辆车
        if (strcmp(s1.train_ID, t1.train_ID) < 0) i1++;
        else if (strcmp(s1.train_ID, t1.train_ID) > 0) i2++;
        else if (s1.index >= t1.index) { //同一辆车，但是 s1 应该在 t1 之后
            i1++, i2++; // 注意到，一列火车只会经过站点一次
            //如果s1在t1前，不满足，说明这列火车不可能从s走到t，所以两个都要++，去找下一列火车
        } else { //合法情况
            TimeType start_day = day - s1.leaving_time.get_date(); //要在day这一天上车，对应车次的首发时间
            //也就是从第一站出发的日期
            if (s1.start_sale_time <= start_day && start_day <= s1.end_sale_time) { //能买到
                tickets[++cnt] = Ticket(s1, t1);
            }
            i1++, i2++;
        }
    }

    if (!cnt) return "0"; //无符合条件的车票

    if (type == "time") Sort(tickets, 1, cnt, time_cmp);
    else Sort(tickets, 1, cnt, cost_cmp);

    string output = to_string(cnt);
    for (int i = 1; i <= cnt; ++i) {
        TimeType start_day = day - tickets[i].s.leaving_time.get_date();
        string key = string(tickets[i].s.train_ID) + start_day.transfer();
        vector<int> ans3;
        daytrain_id_to_pos.find_node(key, ans3);
        DayTrain tp_daytrain;
        day_train_data.read(tp_daytrain, ans3[0]);

        string seat = to_string(tp_daytrain.query_seat(tickets[i].s.index, tickets[i].t.index - 1)); //终点站的座位数不影响

        output += "\n" + string(tickets[i].s.train_ID) + " " + string(tickets[i].s.station_name) + " "
                  + (start_day + tickets[i].s.leaving_time).transfer() + " -> "
                  + string(tickets[i].t.station_name) + " "
                  + (start_day + tickets[i].t.arriving_time).transfer() + " "
                  + to_string(tickets[i].cost()) + " " + seat;
    }

    return output;
}

string TrainManagement::query_transfer(Command &line) {
    string opt = line.next_token(), s, t, date, type = "time"; //默认按时间排序
    while (!opt.empty()) {
        if (opt == "-s") s = line.next_token();
        else if (opt == "-t") t = line.next_token();
        else if (opt == "-d") date = line.next_token();
        else if (opt == "-p") type = line.next_token();

        opt = line.next_token();
    }

    if (s == t) return "0"; //起点和终点相同
    TimeType day(date + " 00:00");

    vector<int> all, ans1, ans2;
    //todo:区间查找，查找所有 站点为 s 和 t 的 station 车站
    //因为关键字是直接拼接的，所以不好查，直接暴力遍历
    train_id_to_pos.find_all(all); //全部读取出来，是按照 train_ID 升序排列的
    for (int i = 0; i < all.size(); ++i) {
        Train tp_train;
        train_data.read(tp_train, all[i]);
        string key1 = string(tp_train.train_ID) + s, key2 = string(tp_train.train_ID) + t;

        station_id_to_pos.find_node(key1, ans1);
        station_id_to_pos.find_node(key2, ans2);
    }

    if (ans1.empty() || ans2.empty()) return "0"; //无票
    int cnt = 0;
    Station s1, t1; //起点和终点

    for (int i = 0;i < ans1.size(); ++i) {
        station_data.read(s1, ans1[i]);
        TimeType start_day1 = day - s1.leaving_time.get_date();
        if (start_day1 < s1.start_sale_time || start_day1 > s1.end_sale_time) continue;

        Train train1;
        vector<int> pos;
        train_id_to_pos.find_node(s1.train_ID, pos);
        train_data.read(train1, pos[0]);

        for (int j = 0;j < ans2.size(); ++j) {
            station_data.read(t1, ans2[j]);
            if (!strcmp(s1.train_ID, t1.train_ID)) continue; //换乘要求不同车

            TimeType start_day2 = day - t1.leaving_time.get_date();
        }
    }

    return "0";
}

string TrainManagement::buy_ticket(Command &line, AccountManagement &accounts) {
    string opt = line.next_token(), user_name, train_ID, S, T, date;
    int num, is_pending = 0;
    while (!opt.empty()) {
        if (opt == "-u") user_name = line.next_token();
        else if (opt == "-i") train_ID = line.next_token();
        else if (opt == "-d") date = line.next_token();
        else if (opt == "-n") num = string_to_int(line.next_token());
        else if (opt == "-f") S = line.next_token();
        else if (opt == "-t") T = line.next_token();
        else if (opt == "-q") {
            if (line.next_token() == "true") is_pending = 1;
        }

        opt = line.next_token();
    }

    if (!accounts.login_pool.count(user_name)) return "-1"; //用户未登录

    vector<int> ans;
    Train target_train;
    train_id_to_pos.find_node(train_ID, ans);
    if (ans.empty()) return "-1"; //车次不存在
    train_data.read(target_train, ans[0]);

    if (!target_train.is_released) return "-1"; //车次未发布，不能购票
    if (target_train.total_seat_num < num) return "-1"; //座位不够

    int s = 0, t = 0;
    for (int i = 1;i <= target_train.station_num && !(s && t); ++i) { //求出station index
        if (!strcmp(target_train.stations[i], S.c_str())) s = i;
        if (!strcmp(target_train.stations[i], T.c_str())) t = i;
    }
    if (!s || !t || s >= t) return "-1"; //车站不合要求

    TimeType start_day = TimeType(date + " 00:00") - target_train.leaving_times[s].get_date();
    if (start_day < target_train.start_sale_date || start_day > target_train.end_sale_date) return "-1"; //不在售票日期

    string key = train_ID + start_day.transfer();
    vector<int> ans2;
    daytrain_id_to_pos.find_node(key, ans2);
    DayTrain tp;
    day_train_data.read(tp, ans2[0]);

    int remain_seat = tp.query_seat(s, t - 1);
    if (!is_pending && remain_seat < num) return "-1"; //不补票且座位不够

    int price = target_train.price_sum[t] - target_train.price_sum[s]; //刚好不是 s-1
    int order_ID;
    order_data.get_info(order_ID, 1); //相当于size操作，求有几个元素
    order_ID++; //从1开始

    Order new_order(user_name, train_ID, num, price, order_ID, start_day,
                    target_train.leaving_times[s], target_train.arriving_times[t], Status(success),
                    s, t, target_train.stations[s], target_train.stations[t]);

    //todo: 要修改为pair
    key = user_name + to_string(order_ID);

    if (remain_seat >= num) { //座位足够
        tp.modify_seat(s, t - 1, -num);
        day_train_data.update(tp, ans2[0]);
        int pos = order_data.write(new_order);
        order_id_to_pos.add_node(UllNode(key, pos));
        return to_string((long long) num * price);
    } else { //要候补
        new_order.status = pending;
        PendingOrder pending_order(train_ID, user_name, start_day, num, s, t, order_ID);

        int pos = order_data.write(new_order), pos2 = pending_order_data.write(pending_order);
        order_id_to_pos.add_node(UllNode(key, pos));

        // todo: 要修改，3个关键字复合成的 key
        string key2 = train_ID + start_day.transfer() + to_string(order_ID);
        pending_order_id_to_pos.add_node(UllNode(key2, pos2));
        return "queue";
    }
}

string TrainManagement::query_order(Command &line, AccountManagement &accounts) {
    line.next_token();
    string user_name = line.next_token();
    if (!accounts.login_pool.count(user_name)) return "-1"; //未登录
    int cnt = 0;

    //todo : 修改为区间查找，查找所有关键字包含 user_name 的 order
    vector<int> all;
    order_id_to_pos.find_all(all);
    if (all.empty()) return "0"; //没有订单
    for (int i = 0; i < all.size(); ++i) {
        Order tp_order;
        order_data.read(tp_order, all[i]);
        if (!strcmp(tp_order.user_name, user_name.c_str()))
            orders[++cnt] = tp_order;
    }

    //从新到旧排序 , 可能不需要？
//    Sort(orders, 1, cnt, order_cmp);
    string output = to_string(cnt);
    for (int i = cnt;i >= 1; --i) {
        if (orders[i].status == success) output += "\n [success] ";
        else if (orders[i].status == pending) output += "\n [pending] ";
        else output += "\n [refunded] ";

        output += string(orders[i].train_ID) + " " + string(orders[i].from_station) + " "
                + (orders[i].leaving_time + orders[i].start_day).transfer()+ " -> "
                + string(orders[i].to_station) + " " + (orders[i].arriving_time + orders[i].start_day).transfer()
                + to_string(orders[i].price) + " " + to_string(orders[i].num);
    }
    return output;
}

string TrainManagement::refund_ticket(Command &line, AccountManagement &accounts) {
    string opt = line.next_token(), user_name;
    int x = 1;
    while (!opt.empty()) {
        if (opt == "-u") user_name = line.next_token();
        else if (opt == "-n") x = string_to_int(line.next_token());

        opt = line.next_token();
    }

    if (!accounts.login_pool.count(user_name)) return "-1"; //未登录

    // todo: 区间查询
    int cnt = 0, pos[maxn];
    vector<int> all;
    order_id_to_pos.find_all(all);
    if (all.empty()) return "-1"; //没有订单
    for (int i = 0; i < all.size(); ++i) {
        Order tp_order;
        order_data.read(tp_order, all[i]);
        if (!strcmp(tp_order.user_name, user_name.c_str())) {
            orders[++cnt] = tp_order;
            pos[cnt] = all[i];
        }
    }
    if (x > cnt) return "-1"; //显然超出订单总数
    if (orders[cnt - x + 1].status == refunded) return "-1"; //重复退款

    Order refund_order = orders[cnt - x + 1]; //临时存储
    orders[cnt - x + 1].status = refunded;
    order_data.update(orders[cnt - x + 1], pos[cnt - x + 1]);

    if (orders[cnt - x + 1].status == pending) { //候补的票要修改 pending_database
        vector<int> ans;
        string key = (string)orders[cnt - x + 1].train_ID + orders[cnt - x + 1].start_day.transfer()
                    + to_string(orders[cnt - x + 1].order_ID);
        pending_order_id_to_pos.find_node(key, ans);
        pending_order_id_to_pos.delete_node(UllNode(key, ans[0]));
        pending_order_data.Delete(ans[0]);
        return "0";
    }

    //如果原来的订单success，要修改座位，先增加
    string key = string(refund_order.train_ID) + refund_order.start_day.transfer();
    vector<int> ans;
    daytrain_id_to_pos.find_node(key, ans);
    DayTrain tp_daytrain;
    day_train_data.read(tp_daytrain, ans[0]);
    tp_daytrain.modify_seat(refund_order.from, refund_order.to - 1, refund_order.num);
    day_train_data.update(tp_daytrain, ans[0]);

    //退票后有空缺，判断候补的订单现在是否能买
    int CNT = 0;
    all.clear(); //todo: 同样是区间查找
    pending_order_id_to_pos.find_all(all);
    for (int i = 0; i < all.size(); ++i) {
        PendingOrder tp;
        pending_order_data.read(tp, all[i]);
        if (tp.start_day == refund_order.start_day && !strcmp(tp.train_ID, refund_order.train_ID)) {
            pending_orders[++CNT] = tp;
            pos[CNT] = all[i];
        }
    }

    for (int i = 1;i <= CNT; ++i) {
        //只有候补的起点和终点范围，包括新增加的票的范围，才有意义
        //概括地说，就是不花冤枉钱，不买超出旅程的票
        if (pending_orders[i].from > refund_order.from || pending_orders[i].to < refund_order.to) continue;
        if (tp_daytrain.query_seat(pending_orders[i].from, pending_orders[i].to - 1) >= pending_orders[i].num) {
            //座位足够，而且只能全买
            tp_daytrain.modify_seat(pending_orders[i].from, pending_orders[i].to - 1, -pending_orders[i].num);
            //相应地删除pending_database
            key = string(pending_orders[i].train_ID) + pending_orders[i].start_day.transfer()
                    + to_string(pending_orders[i].order_ID);
            pending_order_id_to_pos.delete_node(UllNode(key, pos[i]));
            pending_order_data.Delete(pos[i]);

            //修改 order 中的状态
            Order success_order;
            key = string(pending_orders[i].user_name) + to_string(pending_orders[i].order_ID);
            vector<int> ans2;
            order_id_to_pos.find_node(key, ans2);
            order_data.read(success_order, ans2[0]);
            success_order.status = success;
            order_data.update(success_order, ans2[0]);
        }
    }
    //把新补票后减少的座位，写入文件中
    day_train_data.update(tp_daytrain, ans[0]);
    return "0";
}

//-------------------todo: special command

string TrainManagement::rollback(Command &line, AccountManagement &accounts) {
    return "0";
}

string TrainManagement::clean(AccountManagement &accounts) {
    accounts.user_data.clear();
    accounts.username_to_pos.clear();
    accounts.login_pool.clear();

    train_data.clear();
    day_train_data.clear();
    station_data.clear();
    order_data.clear();
    pending_order_data.clear();
//    ticket_data.clear();

    train_id_to_pos.clear();
    daytrain_id_to_pos.clear();
    station_id_to_pos.clear();
    order_id_to_pos.clear();
    pending_order_id_to_pos.clear();
//    time_to_pos.clear();
//    cost_to_pos.clear();

    return "0";
}

string TrainManagement::exit(AccountManagement &accounts) {
    accounts.login_pool.clear(); //用户下线
    //其实可以省略，因为在内存中的变量会自动清除？

    printf("bye\n");
    std::exit(0); //可以有 \n 因为直接结束程序
}
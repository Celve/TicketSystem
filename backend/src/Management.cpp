//
// Created by Leonard C on 2022/6/9.
//

#include "Management.h"

const int modify_threshold = 1e6;

namespace thomas {

    template<typename T>
    void Sort(T *a, int l, int r, bool cmp(const T &u, const T &v)) { //对数组
        if (l >= r)
            return;
        int pl = l, pr = r;
        T mid_val = a[(l + r) / 2];
        while (pl <= pr) {
            while (cmp(a[pl], mid_val))
                ++pl;
            while (cmp(mid_val, a[pr]))
                --pr;
            if (pl <= pr)
                std::swap(a[pl], a[pr]), ++pl, --pr;
        }
        if (l < pr)
            Sort(a, l, pr, cmp);
        if (pl < r)
            Sort(a, pl, r, cmp);
    }

    template<typename T>
    void Sort(vector<T> &a, int l, int r,
              bool cmp(const T &u, const T &v)) { //对vector
        if (l >= r)
            return;
        int pl = l, pr = r;
        T mid_val = a[(l + r) / 2];
        while (pl <= pr) {
            while (cmp(a[pl], mid_val))
                ++pl;
            while (cmp(mid_val, a[pr]))
                --pr;
            if (pl <= pr)
                std::swap(a[pl], a[pr]), ++pl, --pr;
        }
        if (l < pr)
            Sort(a, l, pr, cmp);
        if (pl < r)
            Sort(a, pl, r, cmp);
    }

    bool is_legal(const string &s) { //判断该日期是否合法
        int month = string_to_int(s.substr(0, 2));
        //    int day = string_to_int(s.substr(3, 2));
        //    int hour = string_to_int(s.substr(6, 2));
        //    int min = string_to_int(s.substr(9, 2));

        if (month < 6 || month > 8)
            return false;
        return true;
    }

//这是在外部的函数！
//记得 train_ID 是第二关键字
    bool time_cmp(const Ticket &a, const Ticket &b) {
        if (a.time() == b.time())
            return strcmp(a.s.train_ID, b.s.train_ID) < 0;
        return a.time() < b.time();
    }

    bool cost_cmp(const Ticket &a, const Ticket &b) {
        if (a.cost() == b.cost())
            return strcmp(a.s.train_ID, b.s.train_ID) < 0;
        return a.cost() < b.cost();
    }

    bool order_cmp(const Order &a, const Order &b) { //按ID排序 order
        return a.order_ID > b.order_ID;
    }

    bool pending_order_cmp(const PendingOrder &a,
                           const PendingOrder &b) { //按ID排序 order
        return a.order_ID < b.order_ID;               //越早买的越早补票
    }

    bool station_cmp(const std::pair<string, int> &a,
                     const std::pair<string, int> &b) { //按名字排序
        return a.first < b.first;
    }

    void OUTPUT(TrainManagement &all, const string &train_ID) { //用来调试
        using namespace std;

        Train a;
        //        vector<int> tmp1;
        //        all.train_id_to_pos.find_node(train_ID, tmp1);
        //        all.train_data.read(a, tmp1[0]);

        cout << "~~~~ " << a.train_ID << " ~~~~" << endl;
        cout << "stationNum=" << a.station_num << endl;
        for (int i = 1; i <= a.station_num; i++)
            cout << a.stations[i] << " ";
        cout << endl << "seatNum" << endl;

        for (auto i = a.start_sale_date; i <= a.end_sale_date; i += 1440) {
            string key = a.train_ID + i.transfer();

            DayTrain tp;
            vector<int> ans;
            //            all.daytrain_id_to_pos.find_node(key, ans);
            //            all.day_train_data.read(tp, ans[0]);

            for (int j = 1; j < a.station_num; ++j) {
                cout << tp.seat_num[j] << " ";
            }
            cout << endl;
        }

        //    cout << "prices" << endl;
        //    for(int i = 1;i <= a.station_num;i++) cout << a.price_sum[i] << " ";
        //    cout << endl;

        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

//----------------------------------------------class Record_stack

    template<typename T>
    Record_stack<T>::Record_stack(const string &file_name) : stack(file_name) {}

    template<typename T>
    Record_stack<T>::Record_stack(const Record_stack<T> &rhs) : stack(rhs.stack) {}

    template<typename T>
    void Record_stack<T>::init(const string &file_name) {
        stack.Init(file_name);
    }

    template<typename T>
    void Record_stack<T>::add(const int &_type, const int &_time, const T &_data) {
        stack.Push(Record(_type, _time, _data));
    }

    template<typename T>
    typename Record_stack<T>::Record Record_stack<T>::pop() {
        Record temp;
        stack.Pop(temp);
        return temp;
    }

    template<typename T>
    bool Record_stack<T>::empty() {
        return stack.IsEmpty();
    }

    template<typename T>
    size_t Record_stack<T>::size() {
        return stack.Size();
    }

    //----------------------------------------------class AccountManagement

    AccountManagement::AccountManagement() : user_stack("user_stack.db") {
        //    user_data.initialise("user_data");
        //    username_to_pos.init("username_to_pos");
        user_database = new BPlusTreeIndexNTS<String<24>, User, StringComparator<24>>(
                "user_database", cmp1);
        cnt = user_stack.size();
    }

//    AccountManagement::AccountManagement(const string &file_name) {
//        user_database = new BPlusTreeIndexNTS<String<24>, User, StringComparator<24>>(
//                file_name, cmp1);
//    }

    string AccountManagement::add_user(Command &line) {
        string opt = line.next_token(), cur, username, password, name, mail;
        int privilege;
        while (!opt.empty()) {
            if (opt == "-c")
                cur = line.next_token();
            else if (opt == "-u")
                username = line.next_token();
            else if (opt == "-p")
                password = line.next_token();
            else if (opt == "-n")
                name = line.next_token();
            else if (opt == "-m")
                mail = line.next_token();
            else
                privilege = string_to_int(line.next_token());

            opt = line.next_token();
        }

        vector<User> ans;
        user_database->SearchKey(String<24>(username), &ans);

        if (user_database->IsEmpty()) { //首次添加用户
            User u(username, name, mail, password, 10);
            user_database->InsertEntry(String<24>(username), u);
            user_stack.add(0, line.timestamp, u);
            return "0";
        } else {
            //操作失败：未登录/权限不足/用户名已存在
            if (!login_pool.count(cur)) return "user is not logged in";
            else if (login_pool.at(cur) <= privilege) return "permission denied";
            else if (!ans.empty()) return "user already exists";
            else {
                User u(username, name, mail, password, privilege);
                user_database->InsertEntry(String<24>(username), u);
                user_stack.add(0, line.timestamp, u);
                return "0";
            }
        }
    }

    string AccountManagement::login(Command &line) {
        string opt = line.next_token(), username, password;
        while (!opt.empty()) {
            if (opt == "-u")
                username = line.next_token();
            else
                password = line.next_token();

            opt = line.next_token();
        }

        vector<User> ans;
        user_database->SearchKey(String<24>(username), &ans);
        //用户不存在/用户已登录
        if (ans.empty()) return "user does not exist";
        if (login_pool.count(username)) return "user has logged in";

        if (strcmp(ans[0].password, password.c_str())) return "wrong password"; //密码错误

        login_pool.insert(sjtu::pair<string, int>(username, ans[0].privilege));
        return "0";
    }

    string AccountManagement::logout(Command &line) {
        string opt = line.next_token(), username = line.next_token();

        //用户未登录
        if (!login_pool.count(username)) return "user is not logged in";

        login_pool.erase(login_pool.find(username));
        return "0";
    }

    string AccountManagement::modify_profile(Command &line) {
        string opt = line.next_token(), cur, username, password, name, mail;
        int privilege = -1; //记得赋初值！范围是0-10，所以初值为 -1
        while (!opt.empty()) {
            if (opt == "-c")
                cur = line.next_token();
            else if (opt == "-u")
                username = line.next_token();
            else if (opt == "-p")
                password = line.next_token();
            else if (opt == "-n")
                name = line.next_token();
            else if (opt == "-m")
                mail = line.next_token();
            else
                privilege = string_to_int(line.next_token());

            opt = line.next_token();
        }

        vector<User> ans;
        user_database->SearchKey(String<24>(username), &ans);
        if (ans.empty()) return "user does not exist"; // user不存在

        User u = ans[0];
        //cur未登录
        if (!login_pool.count(cur)) return "user is not logged in";
        // cur权限<=u的权限 且 cur != u
        if ((login_pool.at(cur) <= u.privilege) && (cur != username) ||
            privilege >= login_pool.at(cur)) //要修改的权限太大
            return "permission denied";

        //todo: 注意：应该要保存的是修改前的元素
        user_stack.add(2, line.timestamp, u);

        if (!password.empty())
            strcpy(u.password, password.c_str());
        if (!name.empty())
            strcpy(u.name, name.c_str());
        if (!mail.empty())
            strcpy(u.mail_addr, mail.c_str());
        if (privilege != -1) {
            u.privilege = privilege;
            //  实时更新login_pool中u的权限
            if (login_pool.count(username)) {
                login_pool.erase(login_pool.find(username));
                login_pool.insert(sjtu::pair<string, int>(username, privilege));
            }
        }

        //    user_data.update(u, ans[0]);
        user_database->InsertEntry(String<24>(username), u);

        return (string) u.user_name + " " + (string) u.name + " " +
               (string) u.mail_addr + " " + to_string(u.privilege);
    }

    string AccountManagement::query_profile(Command &line) {
        string opt = line.next_token(), cur, username, password, name, mail;
        while (!opt.empty()) {
            if (opt == "-c")
                cur = line.next_token();
            else
                username = line.next_token();

            opt = line.next_token();
        }

        vector<User> ans;
        user_database->SearchKey(String<24>(username), &ans);
        if (ans.empty())
            return "user does not exist"; // u不存在
        User u = ans[0];

        // cur未登录/cur权限<=u的权限 且 cur != u
        if (!login_pool.count(cur) ||
            (login_pool.at(cur) <= u.privilege) && (cur != username))
            return "permission denied";

        return (string) u.user_name + " " + (string) u.name + " " +
               (string) u.mail_addr + " " + to_string(u.privilege);
    }

    AccountManagement::~AccountManagement() { delete user_database; }

//-------------------------------------------------class TrainManagement

    TrainManagement::TrainManagement() : cmp2(2), cmp3(3), cmp4(3), cmp5(2),
                                         train_stack("train_stack.db"), station_stack("station_stack.db"),
                                         daytrain_stack("daytrain_stack.db"), order_stack("order_stack.db"),
                                         pending_order_stack("pending_order_stack.db") {
        //先指定 cmp 的类型

        train_database =
                new BPlusTreeIndexNTS<String<24>, Train, StringComparator<24>>(
                        "train_database", cmp1);
        station_database = new BPlusTreeIndexNTS<DualString<32, 24>, Station,
                DualStringComparator<32, 24>>(
                "station_database", cmp2);
        daytrain_database = new BPlusTreeIndexNTS<StringAny<24, int>, DayTrain,
                StringAnyComparator<24, int>>(
                "daytrain_database", cmp3);
        order_database = new BPlusTreeIndexNTS<StringAny<24, int>, Order,
                StringAnyComparator<24, int>>(
                "order_database", cmp4);
        pending_order_database = new BPlusTreeIndexNTS<StringIntInt<24>, PendingOrder,
                StringIntIntComparator<24>>(
                "pending_order_database", cmp5);

        order_num = order_database->Size();

        cnt = train_stack.size() + station_stack.size() + daytrain_stack.size() + order_stack.size() + pending_order_stack.size();
        extra_info.initialise("extra_info");
        extra_info.read(total, 1);
    }

    TrainManagement::~TrainManagement() {
        delete train_database;
        delete station_database;
        delete daytrain_database;
        delete order_database;
        delete pending_order_database;
    }

    string TrainManagement::add_train(Command &line) {
        string opt = line.next_token(), train_id, stations, prices, type;
        string start_time, travel_times, stop_over_times, sale_date;
        int seat_num = 0, station_num = 0;

        while (!opt.empty()) {
            if (opt == "-i")
                train_id = line.next_token();
            else if (opt == "-n")
                station_num = string_to_int(line.next_token());
            else if (opt == "-m")
                seat_num = string_to_int(line.next_token());
            else if (opt == "-s")
                stations = line.next_token();
            else if (opt == "-p")
                prices = line.next_token();
            else if (opt == "-x")
                start_time = line.next_token();
            else if (opt == "-t")
                travel_times = line.next_token();
            else if (opt == "-o")
                stop_over_times = line.next_token();
            else if (opt == "-d")
                sale_date = line.next_token();
            else
                type = line.next_token();

            opt = line.next_token();
        }

        vector<Train> ans;
        train_database->SearchKey(String<24>(train_id), &ans);
        if (!ans.empty())
            return "train already exists"; // train_ID 已存在，添加失败

        Train new_train(train_id, station_num, seat_num, stations, prices, start_time,
                        travel_times, stop_over_times, sale_date, type);
        train_database->InsertEntry(String<24>(train_id), new_train);
        train_stack.add(0, line.timestamp, new_train);

        return "0";
    }

    string TrainManagement::release_train(Command &line) {
        line.next_token(); //过滤-i
        string t_id = line.next_token();

        vector<Train> ans;
        train_database->SearchKey(String<24>(t_id), &ans);
        if (ans.empty())
            return "train does not exist"; //车次不存在，失败
        Train target_train = ans[0];
        if (target_train.is_released)
            return "train has been released"; //重复发布，失败

        train_stack.add(2, line.timestamp, target_train); //相当于modify
        target_train.is_released = true;
        train_database->InsertEntry(String<24>(t_id), target_train);

        //维护 每天的车次座位数
        for (auto i = target_train.start_sale_date; i <= target_train.end_sale_date;
             i += 1440) {
            DayTrain tp_daytrain; // todo: 可能要开在外面，不然会炸
            strcpy(tp_daytrain.train_ID, target_train.train_ID);
            tp_daytrain.start_day = i;
            for (int j = 1; j <= target_train.station_num; ++j)
                tp_daytrain.seat_num[j] = target_train.total_seat_num;

            daytrain_database->InsertEntry(StringAny<24, int>(t_id, i.get_value()),
                                           tp_daytrain);
            daytrain_stack.add(0, line.timestamp, tp_daytrain);
        }

        //维护 沿途的每个车站
        for (int i = 1; i <= target_train.station_num; ++i) {
            Station tp_station(
                    t_id, target_train.stations[i], target_train.price_sum[i],
                    target_train.start_sale_date, target_train.end_sale_date,
                    target_train.arriving_times[i], target_train.leaving_times[i], i);

            station_database->InsertEntry(
                    DualString<32, 24>(target_train.stations[i], t_id), tp_station);
            station_stack.add(0, line.timestamp, tp_station);
        }

        return "0";
    }

    string TrainManagement::query_train(Command &line) {
        string opt = line.next_token(), t_id, date, output;
        while (!opt.empty()) {
            if (opt == "-i")
                t_id = line.next_token();
            else
                date = line.next_token();

            opt = line.next_token();
        }
        if (!is_legal(date + " 00:00"))
            return "illegal date"; //查询，要判断读入的日期是否合法

        vector<Train> ans;
        TimeType day(date + " 00:00");
        train_database->SearchKey(String<24>(t_id), &ans);
        if (ans.empty())
            return "train does not exist"; //没有车
        Train target_train = ans[0];

        //不在售票日期内，不存在
        if (day < target_train.start_sale_date || day > target_train.end_sale_date)
            return "out of sale date";

        vector<DayTrain> ans2;
        daytrain_database->SearchKey(StringAny<24, int>(t_id, day.get_value()),
                                     &ans2);

        //第一行
        output = t_id + " " + target_train.type + "\n";
        //未发布，则所有票都没卖--->实际上不需要特判
        if (!target_train.is_released) {
            //第二行
            output += string(target_train.stations[1]) + " xx-xx xx:xx -> " +
                      (day + target_train.start_time).transfer() + " 0 " +
                      to_string(target_train.total_seat_num) + "\n";
            for (int i = 2; i <= target_train.station_num - 1; ++i) {
                output += string(target_train.stations[i]) + " " +
                          (day + target_train.arriving_times[i]).transfer() + " -> " +
                          (day + target_train.leaving_times[i]).transfer() + " " +
                          to_string(target_train.price_sum[i]) + " " +
                          to_string(target_train.total_seat_num) + "\n";
            }
            //最后一行
            output += string(target_train.stations[target_train.station_num]) + " " +
                      (day + target_train.arriving_times[target_train.station_num])
                              .transfer() +
                      " -> xx-xx xx:xx " +
                      to_string(target_train.price_sum[target_train.station_num]) +
                      " x";
        } else {
            DayTrain current_daytrain = ans2[0]; //防止 未release, ans2 为空的特殊情况
            //从 current_daytrain 获取实时的座位数
            output += string(target_train.stations[1]) + " xx-xx xx:xx -> " +
                      (day + target_train.start_time).transfer() + " 0 " +
                      to_string(current_daytrain.seat_num[1]) + "\n";
            for (int i = 2; i <= target_train.station_num - 1; ++i) {
                output += string(target_train.stations[i]) + " " +
                          (day + target_train.arriving_times[i]).transfer() + " -> " +
                          (day + target_train.leaving_times[i]).transfer() + " " +
                          to_string(target_train.price_sum[i]) + " " +
                          to_string(current_daytrain.seat_num[i]) + "\n";
            }
            output += string(target_train.stations[target_train.station_num]) + " " +
                      (day + target_train.arriving_times[target_train.station_num])
                              .transfer() +
                      " -> xx-xx xx:xx " +
                      to_string(target_train.price_sum[target_train.station_num]) +
                      " x";
        }
        return output;
    }

    string TrainManagement::delete_train(Command &line) {
        line.next_token();
        string t_id = line.next_token();
        vector<Train> ans;
        train_database->SearchKey(String<24>(t_id), &ans);
        if (ans.empty())
            return "train does not exist"; //不存在，不能删

        Train target_train = ans[0];
        if (target_train.is_released)
            return "train has been released"; //已发布，不能删

        train_stack.add(1, line.timestamp, target_train);
        train_database->DeleteEntry(String<24>(t_id));
        return "0";
    }

    string TrainManagement::query_ticket(Command &line) {
        string opt = line.next_token(), s, t, date, type = "time"; //默认按时间排序
        while (!opt.empty()) {
            if (opt == "-s")
                s = line.next_token();
            else if (opt == "-t")
                t = line.next_token();
            else if (opt == "-d")
                date = line.next_token();
            else if (opt == "-p")
                type = line.next_token();

            opt = line.next_token();
        }
        if (!is_legal(date + " 00:00"))
            return "0"; //查询，要判断读入的日期是否合法

        if (s == t)
            return "0"; //起点等于终点，显然无票
        TimeType day(date + " 00:00");
        vector<Station> ans1, ans2;

        DualStringComparator<32, 24> tp_cmp(1);
        // todo:区间查找，查找所有 站点为 s 和 t 的 station 车站
        station_database->ScanKey(DualString<32, 24>(s, ""), &ans1, tp_cmp);
        station_database->ScanKey(DualString<32, 24>(t, ""), &ans2, tp_cmp);

        if (ans1.empty() || ans2.empty())
            return "0"; //无票
        int cnt = 0;
        Station s1, t1; //起点和终点

        vector<Ticket> tickets;
        for (int i1 = 0, i2 = 0; i1 < ans1.size() && i2 < ans2.size();) {
            s1 = ans1[i1], t1 = ans2[i2];
            //判断是否为同一辆车
            if (strcmp(s1.train_ID, t1.train_ID) < 0)
                i1++;
            else if (strcmp(s1.train_ID, t1.train_ID) > 0)
                i2++;
            else if (s1.index >= t1.index) { //同一辆车，但是 s1 应该在 t1 之后
                i1++, i2++; // 注意到，一列火车只会经过站点一次
                //如果s1在t1前，不满足，说明这列火车不可能从s走到t，所以两个都要++，去找下一列火车
            } else { //合法情况
                TimeType start_day = day - s1.leaving_time.get_date(); //要在day这一天上车，对应车次的首发时间
                //也就是从第一站出发的日期
                if (s1.start_sale_time <= start_day && start_day <= s1.end_sale_time) { //能买到
                    tickets.push_back(Ticket(s1, t1));
                }
                i1++, i2++;
            }
        }
        cnt = tickets.size();
        if (!cnt)
            return "0"; //无符合条件的车票

        if (type == "time")
            Sort(tickets, 0, cnt - 1, time_cmp);
        else
            Sort(tickets, 0, cnt - 1, cost_cmp);

        string output = to_string(cnt);
        for (int i = 0; i <= cnt - 1; ++i) {
            TimeType start_day = day - tickets[i].s.leaving_time.get_date();
            vector<DayTrain> all;
            daytrain_database->SearchKey(
                    StringAny<24, int>(tickets[i].s.train_ID, start_day.get_value()), &all);
            DayTrain tp_daytrain = all[0];

            string seat = to_string(tp_daytrain.query_seat(
                    tickets[i].s.index, tickets[i].t.index - 1)); //终点站的座位数不影响

            output += "\n" + string(tickets[i].s.train_ID) + " " +
                      string(tickets[i].s.station_name) + " " +
                      (start_day + tickets[i].s.leaving_time).transfer() + " -> " +
                      string(tickets[i].t.station_name) + " " +
                      (start_day + tickets[i].t.arriving_time).transfer() + " " +
                      to_string(tickets[i].cost()) + " " + seat;
        }

        return output;
    }

    string TrainManagement::query_transfer(Command &line) {
        string opt = line.next_token(), s, t, date, type = "time",
                output; //默认按时间排序
        while (!opt.empty()) {
            if (opt == "-s")
                s = line.next_token();
            else if (opt == "-t")
                t = line.next_token();
            else if (opt == "-d")
                date = line.next_token();
            else if (opt == "-p")
                type = line.next_token();

            opt = line.next_token();
        }
        if (!is_legal(date + " 00:00"))
            return "0"; //查询，要判断读入的日期是否合法
        if (s == t)
            return "0"; //起点和终点相同
        TimeType day(date + " 00:00");
        int COST = MAX_INT, TIME = MAX_INT, FIRST_TIME = MAX_INT; //用来比较答案
        //总花费，总时间，第一段列车的运行时间（越小表示 Train1_ID 也越小）

        DualStringComparator<32, 24> tp_cmp(1);
        // todo:区间查找，查找所有 站点为 s 和 t 的 station 车站
        vector<Station> ans1, ans2;
        station_database->ScanKey(DualString<32, 24>(s, ""), &ans1, tp_cmp);
        station_database->ScanKey(DualString<32, 24>(t, ""), &ans2, tp_cmp);

        if (ans1.empty() || ans2.empty())
            return "0"; //无票
        int cnt = 0;
        Station s1, t1; //起点，终点

        for (int i = 0; i < ans1.size(); ++i) { //枚举经过起点s1的不同车次
            s1 = ans1[i];
            TimeType start_day1 = day - s1.leaving_time.get_date();
            if (start_day1 < s1.start_sale_time || start_day1 > s1.end_sale_time)
                continue; //买不到票

            vector<Train> all;
            train_database->SearchKey(String<24>(s1.train_ID), &all);
            Train train1 = all[0];

            for (int j = 0; j < ans2.size(); ++j) { //枚举经过终点t1的不同车次
                t1 = ans2[j];
                if (!strcmp(s1.train_ID, t1.train_ID))
                    continue; //换乘要求不同车次

                vector<Train> pos2;
                train_database->SearchKey(String<24>(t1.train_ID), &pos2);
                Train train2 = pos2[0]; //到达的车次

                //把可能途径的车站全部读取出来，方便查询
                //注意循环的范围
                int cnt1 = 0, cnt2 = 0;
                vector<std::pair<string, int>> starts, ends;
                for (int k = s1.index + 1; k <= train1.station_num; ++k)
                    starts.push_back(std::make_pair(train1.stations[k], k));
                for (int k = 1; k < t1.index; ++k)
                    ends.push_back(std::make_pair(train2.stations[k], k));
                cnt1 = starts.size(), cnt2 = ends.size();
                if (!cnt1 || !cnt2)
                    continue;

                // todo: 或许可以删掉？因为已经按照 station_name 排序
                Sort(starts, 0, cnt1 - 1, station_cmp);
                Sort(ends, 0, cnt2 - 1, station_cmp); //先按车站名称排序，可以加快查找

                //枚举中转站
                for (int i1 = 0, i2 = 0; i1 < cnt1 && i2 < cnt2;) {
                    if (starts[i1].first < ends[i2].first)
                        i1++; //找到相同的一站
                    else if (starts[i1].first > ends[i2].first)
                        i2++;
                    else {
                        int k = starts[i1].second, l = ends[i2].second; //找到中转站
                        i1++, i2++;

                        TimeType fast_start_day2; // train2的最快发车日期
                        //保证第二辆车的上车时间，为第一辆车到达当天
                        if (train1.arriving_times[k].get_time() <=
                            train2.leaving_times[l].get_time()) //当天能赶上
                            fast_start_day2 = start_day1 + train1.arriving_times[k].get_date() -
                                              train2.leaving_times[l].get_date();
                        else //赶不上，多等一天
                            fast_start_day2 = start_day1 + train1.arriving_times[k].get_date() -
                                              train2.leaving_times[l].get_date() + 1440;

                        if (t1.end_sale_time < fast_start_day2)
                            continue; //赶不上买票
                        TimeType start_day2 = std::max(
                                fast_start_day2, t1.start_sale_time); //真正的日期，发车且发售
                        bool updated = false;

                        //按照关键字更新答案
                        if (type == "cost") {
                            if ( //第一关键字cost
                                    (COST > train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                            train2.price_sum[l]) ||
                                    //第二关键字time
                                    ((COST == train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                              train2.price_sum[l]) &&
                                     (TIME > (start_day2 + t1.arriving_time) -
                                             (start_day1 + s1.leaving_time))) ||
                                    //第三关键字 train1_ID
                                    ((COST == train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                              train2.price_sum[l]) &&
                                     (TIME == (start_day2 + t1.arriving_time) -
                                              (start_day1 + s1.leaving_time)) &&
                                     (FIRST_TIME > (train1.arriving_times[k] - s1.leaving_time)))) {
                                COST = train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                       train2.price_sum[l];
                                TIME = (start_day2 + t1.arriving_time) -
                                       (start_day1 + s1.leaving_time);
                                FIRST_TIME = (train1.arriving_times[k] - s1.leaving_time);
                                updated = true;
                            }
                        } else {
                            if ( //第一关键字time
                                    (TIME > (start_day2 + t1.arriving_time) -
                                            (start_day1 + s1.leaving_time)) ||
                                    //第二关键字cost
                                    ((TIME == (start_day2 + t1.arriving_time) -
                                              (start_day1 + s1.leaving_time)) &&
                                     (COST > train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                             train2.price_sum[l])) ||
                                    //第三关键字 train1_ID
                                    ((COST == train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                              train2.price_sum[l]) &&
                                     (TIME == (start_day2 + t1.arriving_time) -
                                              (start_day1 + s1.leaving_time)) &&
                                     (FIRST_TIME > (train1.arriving_times[k] - s1.leaving_time)))) {
                                COST = train1.price_sum[k] - s1.price_sum + t1.price_sum -
                                       train2.price_sum[l];
                                TIME = (start_day2 + t1.arriving_time) -
                                       (start_day1 + s1.leaving_time);
                                FIRST_TIME = (train1.arriving_times[k] - s1.leaving_time);
                                updated = true;
                            }
                        }
                        if (updated) { //如果更新答案，就保存结果
                            output.clear();
                            vector<DayTrain> f1, f2;
                            daytrain_database->SearchKey(
                                    StringAny<24, int>(train1.train_ID, start_day1.get_value()),
                                    &f1);
                            daytrain_database->SearchKey(
                                    StringAny<24, int>(train2.train_ID, start_day2.get_value()),
                                    &f2);
                            DayTrain S = f1[0], T = f2[0]; //读出当前的座位

                            output += string(s1.train_ID) + " " + string(s1.station_name) +
                                      " " + (start_day1 + s1.leaving_time).transfer() + " -> " +
                                      string(train1.stations[k]) + " " +
                                      (start_day1 + train1.arriving_times[k]).transfer() + " " +
                                      to_string(train1.price_sum[k] - s1.price_sum) + " " +
                                      to_string(S.query_seat(s1.index, k - 1)) + "\n";
                            output += string(t1.train_ID) + " " + string(train2.stations[l]) +
                                      " " + (start_day2 + train2.leaving_times[l]).transfer() +
                                      " -> " + string(t1.station_name) + " " +
                                      (start_day2 + t1.arriving_time).transfer() + " " +
                                      to_string(t1.price_sum - train2.price_sum[l]) + " " +
                                      to_string(T.query_seat(l, t1.index - 1));
                        }
                    }
                }
            }
        }
        if (FIRST_TIME != MAX_INT)
            return output;
        return "0";
    }

    string TrainManagement::buy_ticket(Command &line, AccountManagement &accounts) {
        string opt = line.next_token(), user_name, train_ID, S, T, date;
        int num, is_pending = 0;
        while (!opt.empty()) {
            if (opt == "-u")
                user_name = line.next_token();
            else if (opt == "-i")
                train_ID = line.next_token();
            else if (opt == "-d")
                date = line.next_token();
            else if (opt == "-n")
                num = string_to_int(line.next_token());
            else if (opt == "-f")
                S = line.next_token();
            else if (opt == "-t")
                T = line.next_token();
            else if (opt == "-q") {
                if (line.next_token() == "true")
                    is_pending = 1;
            }

            opt = line.next_token();
        }

        if (!accounts.login_pool.count(user_name))
            return "user is not logged in"; //用户未登录

        vector<Train> ans;
        train_database->SearchKey(String<24>(train_ID), &ans);
        if (ans.empty())
            return "train does not exist"; //车次不存在
        Train target_train = ans[0];

        if (!target_train.is_released)
            return "train is not released"; //车次未发布，不能购票
        if (target_train.total_seat_num < num)
            return "seat is not enough"; //座位不够

        int s = 0, t = 0;
        for (int i = 1; i <= target_train.station_num && !(s && t);
             ++i) { //求出station index
            if (!strcmp(target_train.stations[i], S.c_str()))
                s = i;
            if (!strcmp(target_train.stations[i], T.c_str()))
                t = i;
        }
        if (!s || !t || s >= t)
            return "illegal station"; //车站不合要求

        TimeType start_day =
                TimeType(date + " 00:00") - target_train.leaving_times[s].get_date();
        if (start_day < target_train.start_sale_date ||
            start_day > target_train.end_sale_date)
            return "out of sale date"; //不在售票日期

        vector<DayTrain> ans2;
        daytrain_database->SearchKey(
                StringAny<24, int>(train_ID, start_day.get_value()), &ans2);
        DayTrain tp = ans2[0];

        int remain_seat = tp.query_seat(s, t - 1);
        if (!is_pending && remain_seat < num)
            return "seat is not enough"; //不补票且座位不够

        int price = target_train.price_sum[t] - target_train.price_sum[s]; //刚好不是 s-1

        //    order_data.get_info(order_ID, 1); //相当于size操作，求有几个元素
        order_num++; //不能用 get_info
        int order_ID = order_num;

        Order new_order(user_name, train_ID, num, price, order_ID, start_day,
                        target_train.leaving_times[s], target_train.arriving_times[t],
                        Status(success), s, t, target_train.stations[s],
                        target_train.stations[t]);
        //    strcpy(new_order.id, (user_name + to_string(order_ID)).c_str());

        if (remain_seat >= num) { //座位足够
            daytrain_stack.add(2, line.timestamp, tp);

            tp.modify_seat(s, t - 1, -num);
            daytrain_database->InsertEntry(
                    StringAny<24, int>(train_ID, start_day.get_value()), tp);
            order_database->InsertEntry(StringAny<24, int>(user_name, order_ID),
                                        new_order);
            order_stack.add(0, line.timestamp, new_order);

            long long total = 1ll * num * price; //一定要加上 1ll
            return to_string(total);
        } else { //要候补
            new_order.status = pending;
            PendingOrder pending_order(train_ID, user_name, start_day, num, s, t,
                                       order_ID);

            order_database->InsertEntry(StringAny<24, int>(user_name, order_ID),
                                        new_order);
            order_stack.add(0, line.timestamp, new_order);
            pending_order_database->InsertEntry(
                    StringIntInt<24>(train_ID, start_day.get_value(), order_ID),
                    pending_order);
            pending_order_stack.add(0, line.timestamp, pending_order);
            //        cout << "queue" << endl;
            //        OUTPUT(*this, target_train.train_ID);

            return "queue";
        }
    }

    string TrainManagement::query_order(Command &line,
                                        AccountManagement &accounts) {
        line.next_token();
        string user_name = line.next_token();
        if (!accounts.login_pool.count(user_name))
            return "user is not logged in"; //未登录
        int cnt = 0;

        // todo : 修改为区间查找，查找所有关键字包含 user_name 的 order
        vector<Order> orders;
        StringAnyComparator<24, int> tp_cmp(1);
        // todo: 分析真正的含义，只考虑user_name
        order_database->ScanKey(StringAny<24, int>(user_name, 0), &orders, tp_cmp);
        if (orders.empty())
            return "0"; //没有订单

        cnt = orders.size();
        //从新到旧排序（大到小） , 可能不需要？
        // todo: 修改为 bpt 后，按照关键字 Order_ID 读取，就不用排序
        // 事实上，目前的bpt做不到，所以还是要 sort
        Sort(orders, 0, cnt - 1, order_cmp);

        string output = to_string(cnt);
        for (int i = 0; i <= cnt - 1; ++i) {
            if (orders[i].status == success)
                output += "\n[success] ";
            else if (orders[i].status == pending)
                output += "\n[pending] ";
            else
                output += "\n[refunded] ";

            output += string(orders[i].train_ID) + " " +
                      string(orders[i].from_station) + " " +
                      (orders[i].leaving_time + orders[i].start_day).transfer() +
                      " -> " + string(orders[i].to_station) + " " +
                      (orders[i].arriving_time + orders[i].start_day).transfer() + " " +
                      to_string(orders[i].price) + " " + to_string(orders[i].num);
        }
        return output;
    }

    string TrainManagement::refund_ticket(Command &line,
                                          AccountManagement &accounts) {
        string opt = line.next_token(), user_name;
        int x = 1;
        while (!opt.empty()) {
            if (opt == "-u")
                user_name = line.next_token();
            else if (opt == "-n")
                x = string_to_int(line.next_token());

            opt = line.next_token();
        }

        if (!accounts.login_pool.count(user_name))
            return "user is not logged in"; //未登录

        // todo: 区间查询
        int cnt = 0;
        vector<Order> orders;
        StringAnyComparator<24, int> tp_cmp(1);
        // todo: 分析真正的含义，只考虑user_name
        order_database->ScanKey(StringAny<24, int>(user_name, 0), &orders, tp_cmp);
        if (orders.empty())
            return "no orders"; //没有订单
        cnt = orders.size();
        //从新到旧排序（大到小） , 可能不需要？
        // todo: 修改为 bpt 后，按照关键字 Order_ID 读取，就不用排序
        // 事实上，目前的bpt做不到，所以还是要 sort
        Sort(orders, 0, cnt - 1, order_cmp);
        if (x > cnt)
            return "illegal order"; //显然超出订单总数
        x--;           // 1-base--->0-base
        if (orders[x].status == refunded)
            return "order has been refunded"; //重复退款

        Order refund_order = orders[x]; //临时存储
        order_stack.add(2, line.timestamp, refund_order);
        orders[x].status = refunded;
        order_database->InsertEntry(StringAny<24, int>(user_name, orders[x].order_ID),
                                    orders[x]);

        if (refund_order.status == pending) { //候补的票要修改 pending_database
            //            string key = string(refund_order.train_ID) +
            //            refund_order.start_day.transfer()
            //                         + to_string(refund_order.order_ID);
            vector<PendingOrder> tmp;
            pending_order_database->SearchKey(StringIntInt<24>(
                    refund_order.train_ID, refund_order.start_day.get_value(),
                    refund_order.order_ID), &tmp);
            pending_order_stack.add(1, line.timestamp, tmp[0]);

            pending_order_database->DeleteEntry(StringIntInt<24>(
                    refund_order.train_ID, refund_order.start_day.get_value(),
                    refund_order.order_ID));

            //        cout << "0" << endl;
            //        OUTPUT(*this, refund_order.train_ID);
            return "0";
        }

        //如果原来的订单success，要修改座位，增加
        //        string key = string(refund_order.train_ID) +
        //        refund_order.start_day.transfer();
        vector<DayTrain> ans;
        daytrain_database->SearchKey(
                StringAny<24, int>(refund_order.train_ID,
                                   refund_order.start_day.get_value()),
                &ans);
        DayTrain tp_daytrain = ans[0];
        daytrain_stack.add(2, line.timestamp, tp_daytrain); //修改前

        tp_daytrain.modify_seat(refund_order.from, refund_order.to - 1,
                                refund_order.num);

        //退票后有空缺，判断候补的订单现在是否能买
        int CNT = 0;
        // todo: 同样是区间查找
        vector<PendingOrder> pending_orders;
        StringIntIntComparator<24> tp_cmp2(1);
        pending_order_database->ScanKey(
                StringIntInt<24>(refund_order.train_ID,
                                 refund_order.start_day.get_value(),
                                 refund_order.order_ID),
                &pending_orders, tp_cmp2);
        CNT = pending_orders.size();

        // todo: 同理的排序修改，但是是从小到大，因为早买票就早补票
        Sort(pending_orders, 0, CNT - 1, pending_order_cmp);

        for (int i = 0; i <= CNT - 1; ++i) {
            //之前写错了，只要候补订单的区间和退掉的票有交集，就可以买
            if (pending_orders[i].from > refund_order.to ||
                pending_orders[i].to < refund_order.from)
                continue;
            if (tp_daytrain.query_seat(pending_orders[i].from,
                                       pending_orders[i].to - 1) >=
                pending_orders[i].num) {
                //座位足够，而且只能全买
                tp_daytrain.modify_seat(pending_orders[i].from, pending_orders[i].to - 1,
                                        -pending_orders[i].num);
                //相应地删除pending_database
                pending_order_stack.add(1, line.timestamp, pending_orders[i]);
                pending_order_database->DeleteEntry(StringIntInt<24>(
                        pending_orders[i].train_ID, pending_orders[i].start_day.get_value(),
                        pending_orders[i].order_ID));

                //修改 order 中的状态
                //                key = string(pending_orders[i].user_name) +
                //                to_string(pending_orders[i].order_ID);
                vector<Order> tmp;
                order_database->SearchKey(StringAny<24, int>(pending_orders[i].user_name,
                                                             pending_orders[i].order_ID),
                                          &tmp);
                Order success_order = tmp[0];
                order_stack.add(2, line.timestamp, tmp[0]); //修改前
                success_order.status = success;
                order_database->InsertEntry(
                        StringAny<24, int>(pending_orders[i].user_name,
                                           pending_orders[i].order_ID),
                        success_order);
            }
        }
        //把新补票后减少的座位，写入文件中
        daytrain_database->InsertEntry(
                StringAny<24, int>(refund_order.train_ID,
                                   refund_order.start_day.get_value()),
                tp_daytrain);

        //    cout << "0" << endl;
        //    OUTPUT(*this, refund_order.train_ID);
        return "0";
    }

//-------------------todo: special command

    string TrainManagement::rollback(Command &line, AccountManagement &accounts) {
        line.next_token();
        int to = string_to_int(line.next_token()), now = line.timestamp;

        //回滚的时间不存在
        if (to > now) return "illegal time";

        //清空登录池
        accounts.login_pool.clear();

        while (!accounts.user_stack.empty()) {
            auto tp = accounts.user_stack.pop();
            if (tp.time < to) { //防止越界
                accounts.user_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            auto key = String<24>(tp.data.get_id());
            if (!tp.type) accounts.user_database->DeleteEntry(key); //insert
            else accounts.user_database->InsertEntry(key, tp.data); //delete/modify
        }

        while (!train_stack.empty()) {
            auto tp = train_stack.pop();
            if (tp.time < to) {
                train_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            auto key = String<24>(tp.data.get_id());
            if (!tp.type) train_database->DeleteEntry(key);
            else train_database->InsertEntry(key, tp.data);

        }

        while (!station_stack.empty()) {
            auto tp = station_stack.pop();
            if (tp.time < to) {
                station_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            string s0 = tp.data.get_id();
            int x = 0;
            for (int i = 0; i < s0.length(); ++i) {
                if (s0[i] == ' ') {
                    x = i;
                    break;
                }
            }
            auto key = DualString<32, 24>(s0.substr(0, x), s0.substr(x + 1, s0.length() - x - 1));
            if (!tp.type) station_database->DeleteEntry(key);
            else station_database->InsertEntry(key, tp.data);
        }

        while (!daytrain_stack.empty()) {
            auto tp = daytrain_stack.pop();
            if (tp.time < to) {
                daytrain_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            string s0 = tp.data.get_id();
            int x = 0;
            for (int i = 0; i < s0.length(); ++i) {
                if (s0[i] == ' ') {
                    x = i;
                    break;
                }
            }
            auto key = StringAny<24, int>(s0.substr(0, x),
                                          string_to_int(s0.substr(x + 1, s0.length() - x - 1)));
            if (!tp.type) daytrain_database->DeleteEntry(key);
            else daytrain_database->InsertEntry(key, tp.data);
        }

        while (!order_stack.empty()) {
            auto tp = order_stack.pop();
            if (tp.time < to) {
                order_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            string s0 = tp.data.get_id();
            int x = 0;
            for (int i = 0; i < s0.length(); ++i) {
                if (s0[i] == ' ') {
                    x = i;
                    break;
                }
            }
            auto key = StringAny<24, int>(s0.substr(0, x),
                                          string_to_int(s0.substr(x + 1, s0.length() - x - 1)));
            if (!tp.type) order_database->DeleteEntry(key);
            else order_database->InsertEntry(key, tp.data);
        }

        while (!pending_order_stack.empty()) {
            auto tp = pending_order_stack.pop();
            if (tp.time < to) {
                pending_order_stack.add(tp.type, tp.time, tp.data);
                break;
            }
            string s0 = tp.data.get_id();
            int x1 = 0, x2 = 0;
            for (int i = 0; i < s0.length(); ++i) {
                if (s0[i] == ' ') {
                    x1 = i;
                    break;
                }
            }
            for (int i = s0.length() - 1; i >= 0; --i) {
                if (s0[i] == ' ') {
                    x2 = i;
                    break;
                }
            }
            auto key = StringIntInt<24>(s0.substr(0, x1),
                                        string_to_int(s0.substr(x1 + 1, x2 - x1 - 1)),
                                        string_to_int(s0.substr(x2 + 1, s0.length() - x2 - 1)));
            if (!tp.type) pending_order_database->DeleteEntry(key);
            else pending_order_database->InsertEntry(key, tp.data);
        }

        return "0";
    }

    string TrainManagement::clean(AccountManagement &accounts) {
        //        accounts.user_data.clear();
        //        accounts.username_to_pos.clear();
        accounts.user_database->Clear();
        accounts.login_pool.clear();

        train_database->Clear();
        daytrain_database->Clear();
        station_database->Clear();
        order_database->Clear();
        pending_order_database->Clear();

        return "0";
    }

    string TrainManagement::exit(AccountManagement &accounts) {
        accounts.login_pool.clear(); //用户下线
        //其实可以省略，因为在内存中的变量会自动清除？

        printf("bye\n");
        std::exit(0); //可以有 \n 因为直接结束程序
    }

    string TrainManagement::Export(AccountManagement &accounts, int num) {
        total++;
        extra_info.update(total, 1);
        if (num == -1) num = total;

        string tp = std::to_string(num);

        //新建备份版本，若已存在，则报错
        if (system(string("mkdir backup/" + tp).c_str())) return "backup/" + tp + " already exists!";

        system(string("cp daytrain_database.db backup/" + tp + "/daytrain_database.db").c_str());
        system(string("cp order_database.db backup/" + tp + "/order_database.db").c_str());
        system(string("cp pending_order_database.db backup/" + tp + "/pending_order_database.db").c_str());
        system(string("cp station_database.db backup/" + tp + "/station_database.db").c_str());
        system(string("cp train_database.db backup/" + tp + "/train_database.db").c_str());
        system(string("cp user_database.db backup/" + tp + "/user_database.db").c_str());

        system(string("cp daytrain_stack.db backup/" + tp + "/daytrain_stack.db").c_str());
        system(string("cp order_stack.db backup/" + tp + "/order_stack.db").c_str());
        system(string("cp pending_order_stack.db backup/" + tp + "/pending_order_stack.db").c_str());
        system(string("cp station_stack.db backup/" + tp + "/station_stack.db").c_str());
        system(string("cp train_stack.db backup/" + tp + "/train_stack.db").c_str());
        system(string("cp user_stack.db backup/" + tp + "/user_stack.db").c_str());

        system(string("cp extra_info backup/" + tp + "/extra_info").c_str());

        //修改当前的 cnt
        accounts.cnt = accounts.user_stack.size();
        cnt = train_stack.size() + station_stack.size() + daytrain_stack.size() + order_stack.size() + pending_order_stack.size();

        return "export backup/" + tp + " successfully!";
    }

    string TrainManagement::Import(AccountManagement &accounts, const int &num) {
        string tp = std::to_string(num);

        system(string("cp backup/" + tp + "/daytrain_database.db daytrain_database.db").c_str());
        system(string("cp backup/" + tp + "/order_database.db order_database.db").c_str());
        system(string("cp backup/" + tp + "/pending_order_database.db pending_order_database.db").c_str());
        system(string("cp backup/" + tp + "/station_database.db station_database.db").c_str());
        system(string("cp backup/" + tp + "/train_database.db train_database.db").c_str());
        system(string("cp backup/" + tp + "/user_database.db user_database.db").c_str());

        system(string("cp backup/" + tp + "/daytrain_stack.db daytrain_stack.db").c_str());
        system(string("cp backup/" + tp + "/order_stack.db order_stack.db").c_str());
        system(string("cp backup/" + tp + "/pending_order_stack.db pending_order_stack.db").c_str());
        system(string("cp backup/" + tp + "/station_stack.db station_stack.db").c_str());
        system(string("cp backup/" + tp + "/train_stack.db train_stack.db").c_str());
        system(string("cp backup/" + tp + "/user_stack.db user_stack.db").c_str());
        system(string("cp backup/" + tp + "/extra_info extra_info").c_str());

        // 修改内存中的数据
//        clean(accounts);
        delete accounts.user_database;
        delete train_database;
        delete station_database;
        delete daytrain_database;
        delete order_database;
        delete pending_order_database;

        accounts.user_database = new BPlusTreeIndexNTS<String<24>, User, StringComparator<24>>(
                "user_database", cmp1);
        train_database =
                new BPlusTreeIndexNTS<String<24>, Train, StringComparator<24>>(
                        "train_database", cmp1);
        station_database = new BPlusTreeIndexNTS<DualString<32, 24>, Station,
                DualStringComparator<32, 24>>(
                "station_database", cmp2);
        daytrain_database = new BPlusTreeIndexNTS<StringAny<24, int>, DayTrain,
                StringAnyComparator<24, int>>(
                "daytrain_database", cmp3);
        order_database = new BPlusTreeIndexNTS<StringAny<24, int>, Order,
                StringAnyComparator<24, int>>(
                "order_database", cmp4);
        pending_order_database = new BPlusTreeIndexNTS<StringIntInt<24>, PendingOrder,
                StringIntIntComparator<24>>(
                "pending_order_database", cmp5);

        accounts.user_stack.init("user_stack");
        train_stack.init("train_stack");
        station_stack.init("station_stack");
        daytrain_stack.init("daytrain_stack");
        order_stack.init("order_stack");
        station_stack.init("station_stack");
        order_num = order_database->Size();

        accounts.cnt = accounts.user_stack.size();
        cnt = train_stack.size() + station_stack.size() + daytrain_stack.size() + order_stack.size() + pending_order_stack.size();
        extra_info.initialise("extra_info");
        extra_info.read(total, 1);

        return "import backup/" + tp + " successfully!";
    }

    string TrainManagement::Backup(AccountManagement &accounts) {
        Export(accounts);
        return "succeed";
    }

    void TrainManagement::Auto_backup(AccountManagement &accounts) {
        long long diff = accounts.user_stack.size() - accounts.cnt;
        diff += train_stack.size() + station_stack.size() + daytrain_stack.size() +
                order_stack.size() + pending_order_stack.size() - cnt;

        if (diff > modify_threshold) { //修改次数超出上界，封装为新的版本
            //找出最新的版本编号
            Export(accounts);
        }
    }

} // namespace thomas
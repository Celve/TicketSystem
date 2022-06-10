#include "TrainSystem.h"

using namespace thomas;

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
    int ans = MAX_INT;
    for (int i = l; i <= r; ++i) ans = std::min(ans, seat_num[i]);
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
    strcpy(id, (_user_name + to_string(order_ID)).c_str());
}

//---------------------------------------------------class PendingOrder

PendingOrder::PendingOrder(const string &_train_ID, const string &_user_name, const TimeType &_start_day,
                           const int &_num, const int &_from, const int &_to, const int &_order_ID)
        : start_day(_start_day), num(_num), from(_from), to(_to), order_ID(_order_ID) { //注意...
    strcpy(train_ID, _train_ID.c_str());
    strcpy(user_name, _user_name.c_str());
    strcpy(id, (_train_ID + _start_day.transfer() + to_string(order_ID)).c_str());
}

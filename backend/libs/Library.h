#ifndef TICKETSYSTEM_LIBRARY_H
#define TICKETSYSTEM_LIBRARY_H

#include <string>
#include <cstring>
#include <ostream>

using std::string;
using std::to_string;
using std::ostream;

extern int string_to_int(const string &s); //防止 multiple definition

static int Month[3] = {30, 31, 31}; //只在这个文件里有用

//注意：对于纯日期，要 + " 00:00"，对于纯时刻，要在前面加上 "06-01 "
class TimeType{
private:
    int minute; //表示当前的时间距离 起始时间2021-6-1 00:00 有多少 分钟

public:
    TimeType() = default;

    TimeType(const int &x) : minute(x) {}

    TimeType(const string &s) { //通过：Month-Day Hour:Minute 字符串来构造
        int month = string_to_int(s.substr(0, 2));
        int day = string_to_int(s.substr(3, 2));
        int hour = string_to_int(s.substr(6, 2));
        int min = string_to_int(s.substr(9, 2));

        minute = 0;
        for (int i = 0;i < month - 6; ++i) minute += Month[i] * 1440;
        minute += (day - 1) * 1440;
        minute += hour * 60 + min;
    }

    string transfer() const { //转化为形如：Month-Day Hour:Minute 的字符串
        string month, day, hour, min;
        int t = minute % 1440, cnt = 6;

        min = to_string(t % 60);
        hour = to_string(t / 60);

        t = minute / 1440; //天数
        t++;

        for (int i = 0; i < 3 && t - Month[i] > 0; ++i) {
            t -= Month[i];
            cnt++;
        }
        month = to_string(cnt);
        day = to_string(t);

        if (min.length() == 1) min = "0" + min; //补零
        if (hour.length() == 1) hour = "0" + hour;
        if (day.length() == 1) day = "0" + day;
        if (month.length() == 1) month = "0" + month;
        return month + "-" + day + " " + hour + ":" + min;
    }

    TimeType operator+(const TimeType &rhs) const {
        return TimeType(minute + rhs.minute);
    }

    TimeType operator+(const int &x) const {
        return TimeType(minute + x);
    }

    TimeType operator+=(const int &x) { //要修改本身，不能加 const
        minute += x;
        return *this;
    }

    TimeType operator-=(const int &x) {
        minute -= x;
        return *this;
    }

    int operator-(const TimeType &rhs) const{
        return minute - rhs.minute;
    }

    bool operator==(const TimeType &rhs) const {
        return minute == rhs.minute;
    }

    bool operator!=(const TimeType &rhs) const {
        return !(rhs == *this);
    }

    bool operator<(const TimeType &rhs) const {
        return minute < rhs.minute;
    }

    bool operator>(const TimeType &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const TimeType &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const TimeType &rhs) const {
        return !(*this < rhs);
    }

    friend ostream &operator<<(ostream &os, const TimeType &type) {
        os << type.transfer();
        return os;
    }

    TimeType get_date() const{ //返回当前日期 xx-xx
        return TimeType(minute - minute % 1440);
    }

    TimeType get_time() const{//返回当前时间 xx:xx
        return TimeType(minute % 1440);
    }

    int get_value() const{
        return minute;
    }
};

#endif //TICKETSYSTEM_LIBRARY_H

#ifndef TICKETSYSTEM_COMMAND_H
#define TICKETSYSTEM_COMMAND_H

#include <string>
#include <istream>
#include <ostream>

using std::string;
using std::istream;
using std::ostream;

class Command {
    friend class AccountManagement;

    friend class TrainManagement;

private:
    string buffer = "";//存储字符串的缓冲区
    int cur = 0;//当前指针的位置
    char delimiter = ' ';//分隔符

public:
    int cnt = 0, timestamp = 0;//时间戳

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

#endif //TICKETSYSTEM_COMMAND_H

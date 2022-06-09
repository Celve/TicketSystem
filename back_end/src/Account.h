#ifndef TICKETSYSTEM_ACCOUNT_H
#define TICKETSYSTEM_ACCOUNT_H

#include "Command.h"
#include "FileStorage.h"
//#include "BPlusTree.h"
//#include "ull.h"
#include "map.hpp"
#include "Library.h"

#include <fstream>
#include <cstdio>


class User{
    friend class AccountManagement;
private:
    char user_name[25], password[35], mail_addr[35], name[32]; //数组大小要略大于实际长度
    int privilege; //优先等级
//    vector<Tickets> ticket; //购买的车票

public:
    User() = default; //构造函数
    User(const string &_username, const string &_name, const string &_mail,
         const string &_password, const int &_p);
    bool operator<(const User &rhs) const;
};


#endif //TICKETSYSTEM_ACCOUNT_H

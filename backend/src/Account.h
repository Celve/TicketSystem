#ifndef TICKETSYSTEM_ACCOUNT_H
#define TICKETSYSTEM_ACCOUNT_H

#include "Command.h"
#include "Library.h"
#include "map.hpp"

#include <cstdio>
#include <fstream>

namespace thomas {
    class User {
        friend class AccountManagement;

    private:
        char user_name[22], password[32], mail_addr[32],
                name[32];  //数组大小要略大于实际长度，而且不能大太多。否则disk_limit
        int privilege; //优先等级
        //    vector<Tickets> ticket; //购买的车票

    public:
        User() = default; //构造函数
        User(const string &_username, const string &_name, const string &_mail,
             const string &_password, const int &_p);

        bool operator<(const User &rhs) const;
        string get_id();
    };
} // namespace thomas

#endif // TICKETSYSTEM_ACCOUNT_H

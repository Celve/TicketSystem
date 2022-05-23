#ifndef TICKETSYSTEM_ACCOUNT_H
#define TICKETSYSTEM_ACCOUNT_H

#include "Command.h"
#include "FileStorage.h"
//#include "BPlusTree.h"
#include "ull.h"
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

class AccountManagement{
    friend class TrainManagement;
private:
    sjtu::map<string, int> login_pool;//登录池,username -> privilege
    //可以用二叉查找树实现(红黑树)，以加快查询速度
    //更新：不能保证实时修改，所以login_pool里面的权限没有用...
    MemoryRiver<User> user_data;//保存数据
    Ull username_to_pos; //索引，暂时用 Ull 完成，最后要改为 BpTree

public:
    AccountManagement();
    AccountManagement(const string &file_name);

    string add_user(Command &line); //增加用户
    string login(Command &line); //登录
    string logout(Command &line); //登出
    string query_profile(Command &line); //查询用户信息
    string modify_profile(Command &line); //修改用户信息
};

#endif //TICKETSYSTEM_ACCOUNT_H

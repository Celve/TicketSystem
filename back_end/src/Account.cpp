#include "Account.h"

//-------------------------------------------------class User

User::User(const string &_username, const string &_name, const string &_mail,
           const string &_password, const int &_p) : privilege(_p) {
    strcpy(user_name, _username.c_str());
    strcpy(name, _name.c_str());
    strcpy(mail_addr, _mail.c_str());
    strcpy(password, _password.c_str());
}

bool User::operator<(const User &rhs) const {
    return strcmp(user_name, rhs.user_name) < 0;
}


//----------------------------------------------class AccountManagement

AccountManagement::AccountManagement() {
    user_data.initialise("user_data");
    username_to_pos.init("username_to_pos");
}

AccountManagement::AccountManagement(const string &file_name) {
    user_data.initialise("user_data");
    username_to_pos.init(file_name);
}

string AccountManagement::add_user(Command &line) {
//    line.set_delimiter(' ');
    string opt = line.next_token(), cur, username, password, name, mail;
    int privilege, num = 0;
    while (!opt.empty()) {
        if (opt == "-c") cur = line.next_token();
        else if (opt == "-u") username = line.next_token();
        else if (opt == "-p") password = line.next_token();
        else if (opt == "-n") name = line.next_token();
        else if (opt == "-m") mail = line.next_token();
        else privilege = string_to_int(line.next_token());

        opt = line.next_token();
    }

    user_data.get_info(num, 1);
    vector<int> ans;
    username_to_pos.find_node(username, ans);

    if (!num) { //首次添加用户
        User u(username, name, mail, password, 10);
        int pos = user_data.write(u);
        username_to_pos.add_node(UllNode(username, pos));
        return "0";
    } else {
        //操作失败：未登录/权限不足/用户名已存在
        if (!login_pool.count(cur) || login_pool.at(cur) <= privilege || !ans.empty()) {
            return "-1";
        } else {
            User u(username, name, mail, password, privilege);
            int pos = user_data.write(u);
            username_to_pos.add_node(UllNode(username, pos));
            return "0";
        }
    }
}

string AccountManagement::login(Command &line) {
    string opt = line.next_token(), username, password;
    while (!opt.empty()) {
        if (opt == "-u") username = line.next_token();
        else password = line.next_token();

        opt = line.next_token();
    }

    vector<int> ans;
    username_to_pos.find_node(username, ans);
    //用户不存在/用户已登录
    if (ans.empty() || login_pool.count(username)) return "-1";

    User u;
    user_data.read(u, ans[0]);
    if (strcmp(u.password, password.c_str())) return "-1"; //密码错误

    login_pool.insert(sjtu::pair<string, int>(username, u.privilege));
    return "0";
}

string AccountManagement::logout(Command &line) {
    string opt = line.next_token(), username = line.next_token();

    //用户未登录
    if (!login_pool.count(username)) return "-1";

    login_pool.erase(login_pool.find(username));
    return "0";
}

string AccountManagement::modify_profile(Command &line) {
    string opt = line.next_token(), cur, username, password, name, mail;
    int privilege = 0; //记得赋初值！
    while (!opt.empty()) {
        if (opt == "-c") cur = line.next_token();
        else if (opt == "-u") username = line.next_token();
        else if (opt == "-p") password = line.next_token();
        else if (opt == "-n") name = line.next_token();
        else if (opt == "-m") mail = line.next_token();
        else privilege = string_to_int(line.next_token());

        opt = line.next_token();
    }

    vector<int> ans;
    username_to_pos.find_node(username, ans);
    if (ans.empty()) return "-1";                   //u不存在
    User u;
    user_data.read(u, ans[0]);

    //cur未登录/cur权限<=u的权限 且 cur != u
    if (!login_pool.count(cur) || (login_pool.at(cur) <= u.privilege) && (cur != username)
        || privilege >= login_pool.at(cur) )
        return "-1";


    if (!password.empty()) strcpy(u.password, password.c_str());
    if (!name.empty()) strcpy(u.name, name.c_str());
    if (!mail.empty()) strcpy(u.mail_addr, mail.c_str());
    if (privilege) u.privilege = privilege;

    user_data.update(u, ans[0]);
    return (string)u.user_name + " " + (string)u.name + " " + (string)u.mail_addr + " " + to_string(u.privilege);
}

string AccountManagement::query_profile(Command &line) {
    string opt = line.next_token(), cur, username, password, name, mail;
    int privilege, num = 0;
    while (!opt.empty()) {
        if (opt == "-c") cur = line.next_token();
        else username = line.next_token();

        opt = line.next_token();
    }

    User u;
    vector<int> ans;
    username_to_pos.find_node(username, ans);
    if (ans.empty()) return "-1";                   //u不存在
    user_data.read(u, ans[0]);

    //cur未登录/cur权限<=u的权限 且 cur != u
    if (!login_pool.count(cur) || (login_pool.at(cur) <= u.privilege) && (cur != username) )
        return "-1";

    return (string)u.user_name + " " + (string)u.name + " " + (string)u.mail_addr + " " + to_string(u.privilege);
}
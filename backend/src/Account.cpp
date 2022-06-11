#include "Account.h"

//-------------------------------------------------class User

namespace thomas {

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

    string User::get_id() {
        return user_name;
    }
}
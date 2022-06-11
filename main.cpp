#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "Account.h"
#include "Command.h"
#include "Management.h"
#include "TrainSystem.h"

using namespace std;
using namespace thomas;

// vector<Command> commands; //用来回滚
AccountManagement accounts; //声明在外部，防止数组太大，爆栈空间
TrainManagement trains;

//Record_stack records;
//StackManager<Records> records("records"); // 用于 rollback

int main() {
    string input;

    freopen("test_data/roll_back/basic_1/1.in", "r", stdin);
    freopen("output.txt", "w", stdout);

    while (getline(cin, input)) {
        Command cmd(input);
        string time = cmd.next_token();
        int l = time.length();
        cmd.timestamp = string_to_int(time.substr(1, l - 2));
        //        commands.push_back(cmd);

        printf("[%d] ", cmd.timestamp);
//    cout << "[" << cmd.timestamp << "] "; //输出时间戳，方便调试
        string s = cmd.next_token();
        if (s == "add_user")
            printf("%s\n", accounts.add_user(cmd).c_str());
//      cout << accounts.add_user(cmd) << endl;
        else if (s == "login")
            printf("%s\n", accounts.login(cmd).c_str());
//      cout << accounts.login(cmd) << endl;
        else if (s == "logout")
            printf("%s\n", accounts.logout(cmd).c_str());
//      cout << accounts.logout(cmd) << endl;
        else if (s == "query_profile")
            printf("%s\n", accounts.query_profile(cmd).c_str());
//      cout << accounts.query_profile(cmd) << endl;
        else if (s == "modify_profile")
            printf("%s\n", accounts.modify_profile(cmd).c_str());
//      cout << accounts.modify_profile(cmd) << endl;

        else if (s == "add_train")
            printf("%s\n", trains.add_train(cmd).c_str());
//      cout << trains.add_train(cmd) << endl;
        else if (s == "release_train")
            printf("%s\n", trains.release_train(cmd).c_str());
//      cout << trains.release_train(cmd) << endl;
        else if (s == "query_train")
            printf("%s\n", trains.query_train(cmd).c_str());
//      cout << trains.query_train(cmd) << endl;
        else if (s == "delete_train")
            printf("%s\n", trains.delete_train(cmd).c_str());
//      cout << trains.delete_train(cmd) << endl;
        else if (s == "query_ticket")
            printf("%s\n", trains.query_ticket(cmd).c_str());
//      cout << trains.query_ticket(cmd) << endl;
        else if (s == "query_transfer")
            printf("%s\n", trains.query_transfer(cmd).c_str());
//    cout << trains.query_transfer(cmd) << endl; // trains.query_transfer(cmd)

        else if (s == "buy_ticket")
            printf("%s\n", trains.buy_ticket(cmd, accounts).c_str());
//      cout << trains.buy_ticket(cmd, accounts) << endl;
        else if (s == "query_order")
            printf("%s\n", trains.query_order(cmd, accounts).c_str());
//    cout << trains.query_order(cmd, accounts) << endl;
        else if (s == "refund_ticket")
            printf("%s\n", trains.refund_ticket(cmd, accounts).c_str());
//    cout << trains.refund_ticket(cmd, accounts) << endl;
        else if (s == "rollback")
            printf("%s\n", trains.rollback(cmd, accounts).c_str());
//      cout << trains.rollback(cmd, accounts) << endl;
        else if (s == "clean")
            printf("%s\n", trains.clean(accounts).c_str());
//      cout << trains.clean(accounts) << endl;
        else if (s == "exit")
            trains.exit(accounts);
    }

    return 0;
}
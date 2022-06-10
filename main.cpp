#include <iostream>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "Account.h"
#include "TrainSystem.h"
#include "Command.h"
#include "Management.h"

using namespace std;
using namespace thomas;

//vector<Command> commands; //用来回滚
AccountManagement accounts; //声明在外部，防止数组太大，爆栈空间
TrainManagement trains;

int main() {

    string input;

//    freopen("1.in", "r", stdin);
//    freopen("output.txt", "w", stdout);

    while (getline(cin, input)) {
        Command cmd(input);
        string time = cmd.next_token();
        int l = time.length();
        cmd.timestamp = string_to_int(time.substr(1, l - 2));
//        commands.push_back(cmd);

        cout << "[" << cmd.timestamp << "] "; //输出时间戳，方便调试
        string s = cmd.next_token();
        if (s == "add_user") cout << accounts.add_user(cmd) << endl;
        else if (s == "login") cout << accounts.login(cmd) << endl;
        else if (s == "logout") cout << accounts.logout(cmd) << endl;
        else if (s == "query_profile") cout << accounts.query_profile(cmd) << endl;
        else if (s == "modify_profile") cout << accounts.modify_profile(cmd) << endl;

        else if (s == "add_train") cout << trains.add_train(cmd) << endl;
        else if (s == "release_train") cout << trains.release_train(cmd) << endl;
        else if (s == "query_train") cout << trains.query_train(cmd) << endl;
        else if (s == "delete_train") cout << trains.delete_train(cmd) << endl;
        else if (s == "query_ticket") cout << trains.query_ticket(cmd) << endl;
        else if (s == "query_transfer") cout << trains.query_transfer(cmd) << endl; //trains.query_transfer(cmd)

        else if (s == "buy_ticket") cout << trains.buy_ticket(cmd, accounts) << endl;
        else if (s == "query_order") cout << trains.query_order(cmd, accounts) << endl;
        else if (s == "refund_ticket") cout << trains.refund_ticket(cmd, accounts) << endl;
        else if (s == "rollback") cout << trains.rollback(cmd, accounts) << endl;
        else if (s == "clean") cout << trains.clean(accounts) << endl;
        else if (s == "exit") trains.exit(accounts);
    }

    return 0;
}
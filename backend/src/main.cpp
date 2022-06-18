#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>

#include "Account.h"
#include "Command.h"
#include "Management.h"
#include "TrainSystem.h"
#include "thread/thread_pool.h"

#define THRESHOLD 10000

using namespace std;
using namespace thomas;

// vector<Command> commands; //用来回滚
std::vector<int> timestamps;
std::vector<std::future<std::string>> results;

void Output() {
  for (int i = 0; i < results.size(); ++i) {
    printf("[%d] %s\n", timestamps[i], results[i].get().c_str());
  }
  results.clear();
  timestamps.clear();
}

int main() {
  ThreadPool *thread_pool = new ThreadPool(8);
  AccountManagement *accounts =
      new AccountManagement(thread_pool); //声明在外部，防止数组太大，爆栈空间
  TrainManagement *trains = new TrainManagement(thread_pool);
  string input;

  //    freopen("test_data/normal/pressure_1_easy/2.in", "r", stdin);
  //    freopen("output.txt", "w", stdout);
  auto begin = std::chrono::system_clock::now();

  while (getline(cin, input)) {
    Command cmd(input);
    string time = cmd.next_token();
    int l = time.length();
    cmd.timestamp = string_to_int(time.substr(1, l - 2));
    //        commands.push_back(cmd);
    timestamps.push_back(cmd.timestamp);

    // cout << "[" << cmd.timestamp << "] "; //输出时间戳，方便调试
    string s = cmd.next_token();
    if (s == "add_user")
      results.push_back(accounts->add_user(cmd));
    //      cout << accounts->add_user(cmd) << endl;
    else if (s == "login")
      results.push_back(accounts->login(cmd));
    //      cout << accounts->login(cmd) << endl;
    else if (s == "logout")
      results.push_back(accounts->logout(cmd));
    //      cout << accounts->logout(cmd) << endl;
    else if (s == "query_profile")
      results.push_back(accounts->query_profile(cmd));
    //      cout << accounts->query_profile(cmd) << endl;
    else if (s == "modify_profile")
      results.push_back(accounts->modify_profile(cmd));
    //      cout << accounts->modify_profile(cmd) << endl;

    else if (s == "add_train")
      results.push_back(trains->add_train(cmd));
    //      cout << trains->add_train(cmd) << endl;
    else if (s == "release_train")
      results.push_back(trains->release_train(cmd));
    //      cout << trains->release_train(cmd) << endl;
    else if (s == "query_train")
      results.push_back(trains->query_train(cmd));
    //      cout << trains->query_train(cmd) << endl;
    else if (s == "delete_train")
      results.push_back(trains->delete_train(cmd));
    //      cout << trains->delete_train(cmd) << endl;
    else if (s == "query_ticket")
      results.push_back(trains->query_ticket(cmd));
    //      cout << trains->query_ticket(cmd) << endl;
    else if (s == "query_transfer")
      results.push_back(trains->query_transfer(cmd));
    //    cout << trains->query_transfer(cmd) << endl; //
    //    trains->query_transfer(cmd)

    else if (s == "buy_ticket")
      results.push_back(trains->buy_ticket(cmd, *accounts));
    //      cout << trains->buy_ticket(cmd, *accounts) << endl;
    else if (s == "query_order")
      results.push_back(trains->query_order(cmd, *accounts));
    //    cout << trains->query_order(cmd, *accounts) << endl;
    else if (s == "refund_ticket")
      results.push_back(trains->refund_ticket(cmd, *accounts));
    //    cout << trains->refund_ticket(cmd, *accounts) << endl;
    else if (s == "rollback")
      results.push_back(trains->rollback(cmd, *accounts));
    //      cout << trains->rollback(cmd, *accounts) << endl;
    else if (s == "clean")
      results.push_back(trains->clean(*accounts));
    //      cout << trains->clean(*accounts) << endl;
    else if (s == "exit") {
      Output();
      printf("[%d] ", cmd.timestamp);
      auto end = std::chrono::system_clock::now();
      cerr << 1.0 * (end - begin).count() / 1e9 << endl;
      trains->exit(*accounts);
    }
    if (results.size() == THRESHOLD) {
      Output();
    }
  }

  return 0;
}
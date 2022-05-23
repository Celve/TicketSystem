#include "Library.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

int main() {

    string s1, s2;
    getline(cin, s1);
    getline(cin, s2);

    TimeType t1(s1), t2(s2);
//    cout << t1.minute << ' ' << t2.minute << endl;
    cout << t1 << ' ' << t2 << endl << endl;

    for (auto i = t1; i <= t2; i += 1440) {
        cout << i.transfer() << endl;
    }

    return 0;
}
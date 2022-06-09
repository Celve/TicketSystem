#include "Library.h"

int string_to_int(const string &s){
    int t = 0, l = s.length();
    for (int i = 0;i < l; ++i)
        t = t * 10 + s[i] - '0';
    return t;
}
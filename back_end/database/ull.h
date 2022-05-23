#pragma region HEADER

#define ULL_EXPORT
#ifdef ULL_EXPORT
#define ULL_API __declspec(dllexport)
#else
#define ULL_API __declspec(dllimport)
#endif

#ifndef ULL_H
#define ULL_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include "FileStorage.h"

#define LIST_LEN 65
#define BLOCK_SIZE 500
#define BLOCK_SPLIT_THRESHOLD BLOCK_SIZE << 1
#define BLOCK_MERGE_THRESHOLD 250
//从上到下:链表的总长度,块的大小,分裂的max,合并的min
#pragma endregion

using namespace std;
class UllNode {
private:
    int offset, val;//val就是value,是排序的第二关键字
    //offset是节点在文件中的位置
    char str[64];//实际上存储的index,用来比对,第一关键字

public:
    friend class UllBlock;
    friend class Ull;

    UllNode() = default;

    UllNode(const std::string &arg1, const int &value, const int &arg2 = 0) {
        //arg1是第一关键字,arg2同理
        strcpy(str, arg1.c_str());
        //注意转换，存储的是char[]，读入的是string
        val = value;
        offset = arg2;
    }

    bool operator<(const UllNode &x) const {//重载小于号,用于lower_bound
        if (strcmp(str, x.str) == 0) return val < x.val;
        else return strcmp(str, x.str) < 0;
    }

    bool operator>(const UllNode &x) const {
        if (strcmp(str, x.str) == 0) return val > x.val;
        else return strcmp(str, x.str) > 0;
    }

    bool operator>=(const UllNode &x) const {
        return !(*this < x);
    }

    bool operator<=(const UllNode &x) const {
        return !(*this > x);
    }

    bool operator==(const UllNode &x) const {
        return (strcmp(str, x.str) == 0) && (val == x.val);
    }

    bool operator!=(const UllNode &x) const {
        return !(*this == x);
    }

    UllNode &operator=(const UllNode &right) {
        strcpy(str, right.str);
        offset = right.offset;
        val = right.val;
        return *this;
    }
};

class UllBlock {
private:
    int nxt, pre, pos;//指针,pos是块首的位置
    int siz;//siz是块的大小
    UllNode array[(BLOCK_SIZE << 1) + 10];//最大空间要2倍

public:
    friend class Ull;

    UllBlock() {
        nxt = pre = siz = 0;
        pos = 0;
    }

    UllBlock(const int _siz, const UllNode *_array) {
        siz = _siz;
        memcpy(array, _array, _siz * sizeof(UllNode));
    }

    UllBlock &operator=(const UllBlock &right) {
        nxt = right.nxt;
        pre = right.pre;
        pos = right.pos;
        siz = right.siz;
        memcpy(array, right.array, siz * sizeof(UllNode));
        return *this;
    }

    UllNode front() {
        return array[0];
    }

    UllNode back() {//注意到,判断是否在块中,要与最后一个最大的元素比较
        if (!siz) return array[0];
        return array[siz - 1];
    }

    bool add(const UllNode &x) {//在块中新添加元素x,0说明合法
        int pos = std::lower_bound(array, array + siz, x) - array;
        //二分查找第一个不小于x的元素，返回的是指针(位置)
        if (array[pos] == x) return 1;//已经存在,不合法
        siz++;
        for (int i = siz - 1; i > pos; --i)
            array[i] = array[i - 1];//后方的元素向后移一位
        array[pos] = x;
        return siz >= (BLOCK_SIZE << 1);//判断大小是否超出限制(2倍的长度)，需要split
    }

    bool del(const UllNode &x) {//删除块中的元素x,0说明合法
        int pos = std::lower_bound(array, array + siz, x) - array;
        if (array[pos] != x) return 1;//不存在
        siz--;
        for (int i = pos; i < siz; ++i)
            array[i] = array[i + 1];//向前移动一位
        array[siz] = UllNode();
        return 0;
    }

    void search(const std::string &key, std::vector<int> &temp_array) {
        for (int i = 0; i < siz; ++i) {
            if (array[i].str == key)
                temp_array.push_back(array[i].val);
        }
    }//在当前块中查找键值位key的Node，返回一个存储val的vector
};

class Ull {//专门用来处理与文件交互的操作
//    friend class AccountManagement;
//    friend class TrainManagement;
//    friend class LogManagement;
private:
    const std::string file_name;
    int num, fpos;//块的个数和头指针
    //注意:把num存到了文件的头部,需要每次get/write
    MemoryRiver<UllBlock> block_list;

    inline void del_block(const int &pos) {
    }//删除第pos块

public:
    //文件读入,arg就是filename,将二者关联起来
    Ull() {
        num = 0;
        fpos = 8;
    }

    Ull(const std::string &arg) {
        block_list.initialise(arg);
        num = 0;
        fpos = 8;
    }

    void init(const std::string &file) {
        block_list.initialise(file);
        num = 0;
        fpos = 8;
    }

    ~Ull() = default;

    void find_node(const std::string &key, std::vector<int> &temp_array) {
        UllBlock tp_block;
        block_list.get_info(num, 1);
        for (int i = fpos; i; i = tp_block.nxt) {
            block_list.read(tp_block, i);
            if (key >= tp_block.front().str && key <= tp_block.back().str) {
                tp_block.search(key, temp_array);
            }
        }
    }

    void add_node(const UllNode &node) {
        UllBlock new_block;
        block_list.get_info(num, 1);
        if (!num) {//第一次
            new_block.siz = 1;
            new_block.array[0] = node;
            new_block.pos = block_list.write(new_block);
            block_list.update(new_block, new_block.pos);
            //注意,要更新到文件里
            //以下两行代码,已经由上述的write在MemoryRiver中实现
//            num++;
//            block_list.write_info(num, 1);
        } else {
            int cnt = 0;//cnt是当前遍历到第几块
            for (int i = fpos; i; i = new_block.nxt) {
                cnt++;
                block_list.read(new_block, i);
                if (node < new_block.back() || cnt == num) {
                    new_block.add(node);
                    block_list.update(new_block, i);
                    break;
                }
            }
            if (new_block.siz > BLOCK_SPLIT_THRESHOLD)
                split_block(new_block.pos);
        }
    }

    void delete_node(const UllNode &node) {
        block_list.get_info(num, 1);
        UllBlock tp_block;
        for (int i = fpos; i; i = tp_block.nxt) {
            block_list.read(tp_block, i);
            //可能会发生跨越情况
            if (node < tp_block.front() || node > tp_block.back())
                continue;
            else {
                tp_block.del(node);
                block_list.update(tp_block, i);
                if (node > tp_block.back())
                    break;
            }
        }
        if (tp_block.siz <= BLOCK_MERGE_THRESHOLD)
            merge_block(tp_block.pos);
    }

    void split_block(const int &pos) {
        UllBlock tp_block, new_block;
        //在下方的write中已经更新
//        block_list.get_info(num, 1);
//        num++;
//        block_list.write_info(num, 1);

        block_list.read(tp_block, pos);

        int len = tp_block.siz / 2;
        new_block.siz = tp_block.siz - len;
        memcpy(new_block.array, tp_block.array + len, new_block.siz * sizeof(UllNode));
        tp_block.siz = len;

        new_block.pre = tp_block.pos;//新的块接到后面
        new_block.nxt = tp_block.nxt;
        new_block.pos = block_list.write(new_block);
        tp_block.nxt = new_block.pos;

        block_list.update(tp_block, tp_block.pos);
        block_list.update(new_block, new_block.pos);
    }

    void merge_block(const int &pos) {//把pos合并到pre
        if (pos == 8) return;//头节点
        UllBlock tp_block, pre_block, nxt_block;

//        block_list.get_info(num, 1);
//        num--;
//        block_list.write_info(num, 1);

        block_list.read(tp_block, pos);
        block_list.read(pre_block, tp_block.pre);

        if (tp_block.nxt) {//不是尾节点
            block_list.read(nxt_block, tp_block.nxt);
            nxt_block.pre = pre_block.pos;
            block_list.update(nxt_block, nxt_block.pos);
        }

        pre_block.nxt = tp_block.nxt;
        memcpy(pre_block.array + pre_block.siz, tp_block.array, tp_block.siz * sizeof(UllNode));
        pre_block.siz += tp_block.siz;

        block_list.update(pre_block, pre_block.pos);
        block_list.Delete(tp_block.pos);

        if (pre_block.siz >= BLOCK_MERGE_THRESHOLD)
            split_block(pre_block.pos);
    }

    //返回所有UllNode的val(实际上就是pos)
    void find_all(vector<int> &ans){
        UllBlock tp_block;
        for (int i = fpos; i ; i = tp_block.nxt) {
            block_list.read(tp_block, i);
            for (int j = 0; j < tp_block.siz; ++j) {
                ans.push_back(tp_block.array[j].val);
            }
        }
    }

    void clear() {
        block_list.clear();
    }

};


#endif //ULL_H

/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include <iostream>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:

        typedef pair<const Key, T> value_type;
        enum COLOR {
            RED, BLACK
        };

        struct NODE {
            value_type *data;
            NODE *left, *right, *fa;
            COLOR color;

            //默认构造函数
            NODE() : data(nullptr), left(nullptr), right(nullptr), fa(nullptr), color(RED) {};

            NODE(const value_type &_data, COLOR c = RED, NODE *f = nullptr, NODE *lt = nullptr, NODE *rt = nullptr)
                    : fa(f), left(lt), right(rt), color(c) {
                data = new value_type(_data);
            }

            NODE(const Key &_key, const T &_T, COLOR c = RED, NODE *f = nullptr, NODE *lt = nullptr, NODE *rt = nullptr)
                    : fa(f), left(lt), right(rt), color(c) {
                data = new value_type(_key, _T);
            }

            //复制构造函数，新节点的左右孩子在具体函数中处理
            NODE(const NODE &other) : fa(nullptr), left(nullptr), right(nullptr), color(other.color) {
                data = new value_type(*(other.data));
            }

            ~NODE() {
                if (data) {
                    delete data;
                    data = nullptr;
                }
            }
        };

        class RedBlackTree {
        public:
            NODE *root, *end_node, *head, *rear;
            int len;

            //默认构造
            RedBlackTree() : root(nullptr), len(0), head(nullptr), rear(nullptr){
                end_node = new NODE;
            }

            RedBlackTree(NODE *t) : root(t), len(0), head(nullptr), rear(nullptr) {
                end_node = new NODE;
            }

            //复制构造
            RedBlackTree(const RedBlackTree &other) : root(nullptr), len(other.len) {
                clone(root, other.root);
                head = front();  rear = back();
                end_node = new NODE;
            }

            RedBlackTree &operator=(const RedBlackTree &other) {
                //防止自身赋值
                if (this == &other) return *this;
                make_empty(root);
                delete end_node;
                end_node = new NODE;

                clone(root, other.root);
                head = front(); rear = back();
                len = other.len;
                return *this;
            }

            ~RedBlackTree() {
                make_empty(root);
                len = 0;
                head = nullptr; rear = nullptr;
                if (end_node) {
                    delete end_node;
                    end_node = nullptr;
                }
            }

            bool equal(const Key &a, const Key &b) const {
                return !(Compare()(a, b) || Compare()(b, a));
            }

            void clone(NODE *&t, const NODE *p) {
                if (p == nullptr) return;
                t = new NODE(*p); //颜色也要复制
                clone(t->left, p->left);
                clone(t->right, p->right);
                //和二叉链表的区别：要修改父节点
                if (t->left) t->left->fa = t;
                if (t->right) t->right->fa = t;
            }

            //没有删除end_node和len
            void make_empty(NODE *&t) {
//                std::cout << t << std:: endl;
                if (t == nullptr) return;
                //std::cout << t->left << std::endl;
                make_empty(t->left);
                make_empty(t->right);
                delete t;
                t = nullptr;
            }

            NODE *front() const {
                NODE *t = root;
                if (t == nullptr) return nullptr; //特判map为空
                while (t->left) {
                    t = t->left;
                }
                return t;
            }

            NODE *back() const {
                NODE *t = root;
                if (t == nullptr) return nullptr;
                while (t->right) {
                    t = t->right;
                }
                return t;
            }

            NODE *find(const Key &x) const {
//                if (root == nullptr) return nullptr;
//                if (equal(root->data->first, x)) return root;
                NODE *t = root;
                while (t && !equal(t->data->first, x)) {
                    t = (Compare()(x, t->data->first)) ? t->left : t->right;
                }

                return t;
            }

            void insert(const value_type &x) {
                if (root == nullptr) {
                    root = new NODE(x, BLACK); //根节点为黑色
                    head = rear = root;
                    len++;
                    return;
                }
                NODE *t, *parent, *GrandP;
                t = parent = GrandP = root;
                while (1) {
                    //在路径中,自上向下访问
                    if (t) {
                        //直接修改颜色
                        if (t->left && t->left->color == RED &&
                            t->right && t->right->color == RED) {
                            t->left->color = t->right->color = BLACK;
                            t->color = RED;
                            insert_adjust(GrandP, parent, t); //消除连续红节点
                        }
                        GrandP = parent;
                        parent = t;
                        t = (Compare()(x.first, t->data->first)) ? t->left : t->right;
                    } else {
                        //遍历到了叶子节点，就新加入节点
                        t = new NODE(x, RED, parent);
                        len++;
                        if (Compare()(x.first, parent->data->first)) parent->left = t;
                        else parent->right = t;
                        insert_adjust(GrandP, parent, t);
                        root->color = BLACK;
                        head = front(); rear = back();
                        return;
                    }
                }
            }

            void remove(const Key &del) {
                //即最后用替身rep来代替原本t的位置，然后删除t
                NODE *t, *parent, *t2; //t2是t的兄弟节点

                if (root == nullptr) return;
                if (equal(root->data->first, del) && root->left == nullptr && root->right == nullptr) {
                    len--;
                    delete root;
                    root = nullptr;
                    head = rear = nullptr;
                    return;
                }

                t = parent = t2 = root;
                while (1) {
                    remove_adjust(parent, t, t2, del);
                    //删除节点在中间，就把它变成叶节点/非满节点
                    if (equal(t->data->first, del) && t->left && t->right) {
                        NODE *rep = t->right;
                        //删除点的替身rep为t的右子树的最小值
                        while (rep->left) rep = rep->left;
                        //交换颜色
                        COLOR rep_color = rep->color;
                        rep->color = t->color;
                        t->color = rep_color;
                        //先改父亲
                        NODE *t_fa = t->fa, *rep_fa = rep->fa;;
                        if (t_fa) {
                            if (t == t_fa->left) t_fa->left = rep;
                            else t_fa->right = rep;
                        }
                        rep->fa = t_fa;
                        //然后特判，如果替身就是右儿子
                        if (rep_fa == t) {
                            NODE *tmp = t->left;
                            t->left = rep->left;
                            if (rep->left) rep->left->fa = t;
                            t->right = rep->right;
                            if (rep->right) rep->right->fa = t;

                            rep->left = tmp;
                            tmp->fa = rep;
                            rep->right = t;
                            t->fa = rep;
                        } else {
                            rep_fa->left = t;
                            t->fa = rep_fa;

                            NODE *rep_rs = rep->right;
                            rep->left = t->left;
                            t->left->fa = rep;
                            rep->right = t->right;
                            t->right->fa = rep;

                            t->left = nullptr;
                            t->right = rep_rs;
                            if (rep_rs) rep_rs->fa = t;
                        }

                        if (t == root) root = rep;
                        parent = rep;
                        t = rep->right;
                        t2 = rep->left;
                        continue;
                    }
                    //在叶节点/非满节点,t2 = nullptr
                    if (equal(t->data->first, del)) {
                        if (parent->left == t) parent->left = t->right;
                        else parent->right = t->right;
                        len--;
                        delete t;
                        t = nullptr;
                        root->color = BLACK;
                        head = front(); rear = back();
                        return;
                    }

                    parent = t;
                    t = (Compare()(del, t->data->first)) ? t->left : t->right;
                    t2 = (t == parent->left) ? parent->right : parent->left;
                }
            }

            //自上向下调整,使gp->p->t这棵子树合法
            void insert_adjust(NODE *gp, NODE *p, NODE *t) {
                //没有连续的红色,不需要调整
                if (p->color == BLACK) return;
                if (p == root) {
                    p->color = BLACK;
                    return;
                }
                if (gp->left == p) {
                    if (p->left == t) LL(gp);
                    else LR(gp);
                } else {
                    if (p->right == t) RR(gp);
                    else RL(gp);
                }
            }

            //t使当前节点,t2是t的兄弟
            void remove_adjust(NODE *&p, NODE *&t, NODE *&t2, const Key &del) {
//                if (t == nullptr || t2 == nullptr || p == nullptr) return;
                if (t->color == RED) return;
                if (t == root) {
                    if (t->left && t->right && t->left->color == t->right->color) {
                        t->color = RED;
                        t->left->color = t->right->color = BLACK;
                        return;
                    }
                }

                //第一种情况,t有两个黑儿子/1+0
                if ((t->left && t->left->color == BLACK || t->left == nullptr) &&
                    (t->right && t->right->color == BLACK || t->right == nullptr)) {
                    //1.1: t2有两个黑儿子
                    if ((t2->left && t2->left->color == BLACK || t2->left == nullptr) &&
                        (t2->right && t2->right->color == BLACK || t2->right == nullptr)) {
                        p->color = BLACK;
                        t->color = t2->color = RED;
                    } else {  //1.2: t2有红儿子
                        if (p->left == t2) { //左
                            if (t2->left && t2->left->color == RED) { //1.2.1: t2有外侧红儿子
                                LL(p);
                                t->color = t2->color = RED;
                                t2->left->color = p->color = BLACK;
                                t2 = p->left;
                            } else { //1.2.2: t2有内侧红儿子
                                LR(p);
                                p->fa->color = t->color = RED;
                                p->color = t2->color = BLACK;
                                t2 = p->left;
                            }
                        } else { //右
                            if (t2->right && t2->right->color == RED) { //1.2.1: t2有外侧红儿子
                                RR(p);
                                t->color = t2->color = RED;
                                t2->right->color = p->color = BLACK;
                                t2 = p->right;
                            } else { //1.2.2: t2有内侧红儿子
                                RL(p);
                                p->fa->color = t->color = RED;
                                p->color = t2->color = BLACK;
                                t2 = p->right;
                            }
                        }
                    }
                } else {//第二种,t有红儿子
                    if (equal(t->data->first, del)) { //2.1: 找到被删节点 t
                        if (t->left && t->right) { //2.1.1: t有两个儿子
                            if (t->right->color == BLACK) {
                                LL(t);
                                t->color = RED;
                                t->fa->color = BLACK;
                                p = t->fa;
                                t2 = p->left;
                            }
                            return;
                        }
                        if (t->left) { //2.1.2: t只有左孩子
                            LL(t);
                            t->color = RED;
                            t->fa->color = BLACK;
                            p = t->fa;
                            t2 = p->left;
                        } else { //2.1.3: t只有右孩子
                            RR(t);
                            t->color = RED;
                            t->fa->color = BLACK;
                            p = t->fa;
                            t2 = p->right;
                        }
                    } else { //2.2: t不是被删节点
                        //向下走一层
                        p = t;
                        t = (Compare()(del, p->data->first)) ? p->left : p->right;
                        t2 = (t == p->left) ? p->right : p->left;
                        if (t->color == BLACK) { //2.2.1: 新节点t为黑色
                            if (t2 == p->right) {//2.2.1.1: 新节点t是p的左儿子
                                RR(p);
                                p->color = RED;
                                t2->color = BLACK;
                            } else {//2.2.1.2: 新节点t是p的右儿子
                                LL(p);
                                p->color = RED;
                                t2->color = BLACK;
                            }


                            t = p;
                            p = t2;
                            t2 = (t == p->left) ? p->right : p->left;
                            remove_adjust(p, t, t2, del);
                        }
                    }
                }
            }

            void LL(NODE *gp) {
                NODE *p = gp->left, *gp_fa = gp->fa;
                if (gp_fa) { //防止对空节点进一步访问出错
                    if (gp_fa->left == gp) gp_fa->left = p;
                    else gp_fa->right = p;
                }
                p->fa = gp_fa;

                gp->left = p->right;
                if (p->right) p->right->fa = gp;

                p->right = gp;
                gp->fa = p;

                gp->color = RED;
                p->color = BLACK;
                //修改根节点
                while (root->fa) root = root->fa;
            }

            void RR(NODE *gp) {
                NODE *p = gp->right, *gp_fa = gp->fa;
                if (gp_fa) {
                    if (gp_fa->left == gp) gp_fa->left = p;
                    else gp_fa->right = p;
                }
                p->fa = gp_fa;

                gp->right = p->left;
                if (p->left) p->left->fa = gp;

                p->left = gp;
                gp->fa = p;

                gp->color = RED;
                p->color = BLACK;
                while (root->fa) root = root->fa;
            }

            void LR(NODE *gp) {
                NODE *p = gp->left, *t = p->right, *gp_fa = gp->fa;
                if (gp_fa) {
                    if (gp_fa->left == gp) gp_fa->left = t;
                    else gp_fa->right = t;
                }
                t->fa = gp_fa;

                gp->left = t->right;
                if (t->right) t->right->fa = gp;

                p->right = t->left;
                if (t->left) t->left->fa = p;

                t->left = p;
                p->fa = t;
                t->right = gp;
                gp->fa = t;

                gp->color = RED;
                t->color = BLACK;
                while (root->fa) root = root->fa;
            }

            void RL(NODE *gp) {
                NODE *p = gp->right, *t = p->left, *gp_fa = gp->fa;
                if (gp_fa) {
                    if (gp_fa->left == gp) gp_fa->left = t;
                    else gp_fa->right = t;
                }
                t->fa = gp_fa;

                gp->right = t->left;
                if (t->left) t->left->fa = gp;

                p->left = t->right;
                if (t->right) t->right->fa = p;

                t->right = p;
                p->fa = t;
                t->left = gp;
                gp->fa = t;

                gp->color = RED;
                t->color = BLACK;
                while (root->fa) root = root->fa;
            }
        };

        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        RedBlackTree mp;

        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
        class const_iterator;

        class iterator {
            friend class map;

        private:
            NODE *p;
            const map<Key, T, Compare> *id;
        public:
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
//            using difference_type = std::ptrdiff_t;
//            using value_type = T;
//            using pointer = T *;
//            using reference = T &;
//            using iterator_category = std::output_iterator_tag;
            // If you are interested in type_traits, toy_traits_test provides a place to
            // practice. But the method used in that test is old and rarely used, so you
            // may explore on your own.
            // Notice: you may add some code in here and class const_iterator and namespace sjtu to implement toy_traits_test,
            // this part is only for bonus.

            iterator() : p(nullptr), id(nullptr) {}

            iterator(NODE *_p, const map<Key, T, Compare> *_id) : p(_p), id(_id) {}

            iterator(const iterator &other) : p(other.p), id(other.id) {}

            //iter++指的是访问下一个比key大的位置
            iterator operator++(int) {
                iterator t(*this);
                ++(*this);
                return t;
            }

            //TODO ++iter
            iterator &operator++() {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                if (p == id->mp.rear) p = id->mp.end_node;
                else if (p->right) {
                    p = p->right;
                    while (p->left) p = p->left;
                } else {
                    while (p->fa && p->fa->right == p) p = p->fa;
                    p = p->fa;
                }
                return *this;
            }

            iterator operator--(int) {
                iterator t(*this);
                --(*this);
                return t;
            }

            iterator &operator--() {
                if (id->mp.root == nullptr) throw invalid_iterator(); //判断map空
                if (p == id->mp.head || p == nullptr) throw invalid_iterator();
                if (p == id->mp.end_node) p = id->mp.back();
                else if (p->left) {
                    p = p->left;
                    while (p->right) p = p->right;
                } else {
                    while (p->fa && p->fa->left == p) p = p->fa;
                    p = p->fa;
                }
                return *this;
            }

            //返回的是 value_type，不是指针
            value_type &operator*() const {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                return *(p->data);
            }

            bool operator==(const iterator &rhs) const {
                if (id == rhs.id && p == rhs.p) return true;
                else return false;
            }

            bool operator==(const const_iterator &rhs) const {
                if (id == rhs.id && p == rhs.p) return true;
                else return false;
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const

            noexcept {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                return p->data;
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend class map;

        private:
            NODE *p;
            const map<Key, T, Compare> *id;

        public:
            const_iterator() : p(nullptr), id(nullptr) {}

            const_iterator(const const_iterator &other) : p(other.p), id(other.id) {}

            const_iterator(const iterator &other) : p(other.p), id(other.id) {}

            const_iterator(NODE *_p, const map<Key, T, Compare> *_id) : p(_p), id(_id) {}

            const_iterator operator++(int) {
                const_iterator t(*this);
                ++(*this);
                return t;
            }

            const_iterator &operator++() {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                if (p == id->mp.rear) p = id->mp.end_node;
                else if (p->right) {
                    p = p->right;
                    while (p->left) p = p->left;
                } else {
                    while (p->fa && p->fa->right == p) p = p->fa;
                    p = p->fa;
                }
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator t(*this);
                --(*this);
                return t;
            }

            const_iterator &operator--() {
                if (id->mp.root == nullptr) throw invalid_iterator(); //判断map空
                if (p == id->mp.head || p == nullptr) throw invalid_iterator();
                if (p == id->mp.end_node) p = id->mp.back();
                else if (p->left) {
                    p = p->left;
                    while (p->right) p = p->right;
                } else {
                    while (p->fa && p->fa->left == p) p = p->fa;
                    p = p->fa;
                }
                return *this;
            }

            value_type &operator*() const {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                return *(p->data);
            }

            bool operator==(const iterator &rhs) const {
                if (id == rhs.id && p == rhs.p) return true;
                else return false;
            }

            bool operator==(const const_iterator &rhs) const {
                if (id == rhs.id && p == rhs.p) return true;
                else return false;
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept {
                if (p == id->mp.end_node || p == nullptr) throw invalid_iterator();
                return p->data;
            }
        };


        //-------------------------------------------------------------------

        map() = default;

        map(const map &other) : mp(other.mp) {}

        map &operator=(const map &other) {
            mp = other.mp;
            return *this;
        }

        //会自动调用成员 RedBlackTree 的析构函数
        ~map() {}

        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key) {
            NODE *tmp = mp.find(key);
            if (tmp == nullptr) throw index_out_of_bound();
            return tmp->data->second;
        }

        const T &at(const Key &key) const {
            NODE *tmp = mp.find(key);
            if (tmp == nullptr) throw index_out_of_bound();
            return tmp->data->second;
        }

        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T &operator[](const Key &key) {
            NODE *tmp = mp.find(key);
            if (tmp)
                return tmp->data->second;
            else
                return insert(value_type(key, T())).first.p->data->second;
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const {
            NODE *tmp = mp.find(key);
            if (tmp == nullptr) throw index_out_of_bound();
            return tmp->data->second;
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            if (mp.root == nullptr) return iterator(mp.end_node, this);
            return iterator(mp.head, this);
        }

        const_iterator cbegin() const {
            if (mp.root == nullptr) return iterator(mp.end_node, this);
            return const_iterator(mp.head, this);
        }

        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(mp.end_node, this);
        }

        const_iterator cend() const {
            return const_iterator(mp.end_node, this);
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return !(mp.len);
        }

        /**
         * returns the number of elements.
         */
        size_t size() const {
            return mp.len;
        }

        /**
         * clears the contents
         */
        void clear() {
            mp.make_empty(mp.root);
            mp.len = 0;
            mp.head = mp.rear = nullptr;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            NODE *tmp = mp.find(value.first);
            if (tmp) return pair<iterator, bool>(iterator(tmp, this), false);

            mp.insert(value);
            tmp = mp.find(value.first);
            return pair<iterator, bool>(iterator(tmp, this), true);
        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos == end() || pos.p == nullptr || pos.id != this)
                throw invalid_iterator();
            else
                mp.remove(pos.p->data->first);
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            if (mp.find(key)) return 1;
            else return 0;
        }

        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            NODE *t = mp.find(key);
            if (t == nullptr) return end();
            else return iterator(t, this);
        }

        const_iterator find(const Key &key) const {
            NODE *t = mp.find(key);
            if (t == nullptr) return cend();
            else return const_iterator(t, this);
        }

    };

}

#endif

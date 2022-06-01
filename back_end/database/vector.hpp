#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
    template<typename T>
    class vector {
    public:
        int cur_len, max_size;
        T *data;

        /**
         * you can see RandomAccessIterator at CppReference for help.
         */
        class const_iterator;

        class iterator {
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly. In particular, without the following code,
            // @code{std::sort(iter, iter1);} would not compile.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
            friend class vector;

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *p;
            const vector<T> *id;

        public:
            //need constructor !
            iterator() = default;

            iterator(const iterator &rhs) = default;

            iterator(T *_p, const vector<T> *_id) : p(_p), id(_id) {}

            iterator operator+(const int &n) const {
                return iterator(p + n, id);
                //可以直接写 return p + n;有构造函数
            }

            iterator operator-(const int &n) const {
                return iterator(p - n, id);
            }

            int operator-(const iterator &rhs) const {
                if (id != rhs.id) throw invalid_iterator();
                else return p - rhs.p;
            }

            iterator &operator+=(const int &n) {
                p += n;
                return *this;
            }

            iterator &operator-=(const int &n) {
                p -= n;
                return *this;
            }

            iterator operator++(int) {
                iterator t(*this);
                p += 1;
                return t;
            }

            iterator &operator++() {
                p += 1;
                return *this;
            }

            iterator operator--(int) {
                iterator t(*this);
                p -= 1;
                return t;
            }

            iterator &operator--() {
                p -= 1;
                return *this;
            }

            T &operator*() const {
                return *p;
            }

            //reload inequality operators
            bool operator>(const iterator &rhs) const {
                return p > rhs.p;
            }

            bool operator<(const iterator &rhs) const {
                return p < rhs.p;
            }

            bool operator>=(const iterator &rhs) const {
                return p >= rhs.p;
            }

            bool operator<=(const iterator &rhs) const {
                return p <= rhs.p;
            }

            bool operator==(const iterator &rhs) const {
                return rhs.p == p;
            }

            bool operator==(const const_iterator &rhs) const {
                return rhs.p == p;
            }

            bool operator!=(const iterator &rhs) const {
                return rhs.p != p;
            }

            bool operator!=(const const_iterator &rhs) const {
                return rhs.p != p;
            }
        };

        class const_iterator {
            friend class vector;

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *p;
            const vector<T> *id;

        public:
            const_iterator() = default;

            const_iterator(const const_iterator &rhs) = default;

            const_iterator(T *_p, const vector<T> *_id) : p(_p), id(_id) {}
            //const 必须用列表初始化

            operator iterator() {
                return iterator(p, id);
            }

            const_iterator operator+(const int &n) const {
                return const_iterator(p + n, this);
            }

            const_iterator operator-(const int &n) const {
                return const_iterator(p - n, this);
            }

            int operator-(const const_iterator &rhs) const {
                return p - rhs.p;
            }

            const_iterator &operator+=(const int &n) {
                p += n;
                return *this;
            }

            const_iterator &operator-=(const int &n) {
                p -= n;
                return *this;
            }

            const_iterator operator++(int) {
                iterator t(*this);
                p += 1;
                return t;
            }

            const_iterator &operator++() {
                p += 1;
                return *this;
            }

            const_iterator operator--(int) {
                iterator t(*this);
                p -= 1;
                return t;
            }

            const_iterator &operator--() {
                p -= 1;
                return *this;
            }

            T &operator*() const {
                return *p;
            }

            //reload inequality operators
            bool operator>(const const_iterator &rhs) const {
                return p > rhs.p;
            }

            bool operator<(const const_iterator &rhs) const {
                return p < rhs.p;
            }

            bool operator>=(const const_iterator &rhs) const {
                return p >= rhs.p;
            }

            bool operator<=(const const_iterator &rhs) const {
                return p <= rhs.p;
            }

            bool operator==(const iterator &rhs) const {
                return rhs.p == p;
            }

            bool operator==(const const_iterator &rhs) const {
                return rhs.p == p;
            }

            bool operator!=(const iterator &rhs) const {
                return rhs.p != p;
            }

            bool operator!=(const const_iterator &rhs) const {
                return rhs.p != p;
            }
        };

        //---------------------------------------------------------------------------------

        //包含了默认构造
        vector(size_t _max = 10) : max_size(_max), cur_len(0) {//缺省值
            data = (T *) malloc(max_size * sizeof(T));
        }

        vector(const vector &other) : cur_len(other.cur_len), max_size(other.max_size) {
            data = (T *) malloc(max_size * sizeof(T));
            for (int i = 0; i < cur_len; ++i) {
                // data[i] = other.data[i];
                new(data + i) T(other.data[i]);
            }
        }

        ~vector() {
            for (int i = 0;i < cur_len; ++i)//每一项的析构
                data[i].~T();
            free(data);
            cur_len = 0;
            max_size = 0;
        }

        vector &operator=(const vector &other) {
            if (data == other.data) return *this;//防止自我赋值

            for (int i = 0;i < cur_len; ++i)//每一项的析构
                data[i].~T();
            free(data);
            cur_len = other.cur_len;
            max_size = other.max_size;
            data = (T *) malloc(max_size * sizeof(T));
            for (int i = 0; i < cur_len; ++i)
                new(data + i) T(other.data[i]);
            return *this;
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T &at(const size_t &pos) {
            if (pos < 0 || pos >= max_size) throw index_out_of_bound();
            else return data[pos];
        }

        const T &at(const size_t &pos) const {
            if (pos < 0 || pos >= max_size) throw index_out_of_bound();
            else return data[pos];
        }

        T &operator[](const size_t &pos) {
            return at(pos);
        }

        const T &operator[](const size_t &pos) const {
            return at(pos);
        }

        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T &front() const {
            if (!cur_len) throw container_is_empty();
            else return data[0];
        }

        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T &back() const {
            if (!cur_len) throw container_is_empty();
            else return data[cur_len - 1];
        }

        iterator begin() {
            return iterator(data, this);
        }

        const_iterator cbegin() const {
            return const_iterator(data, this);
        }

        iterator end() {
            return iterator(data + cur_len, this);
        }

        const_iterator cend() const {
            return const_iterator(data + cur_len, this);
        }

        bool empty() const {
            return !cur_len;
        }

        size_t size() const {
            return cur_len;
        }

        void clear() {
            for (int i = cur_len - 1;i >= 0;--i)
                data[i].~T();
            cur_len = 0;
        }

        //re-allocate the memory space
        void resize(const int &len) {
            T *new_data = (T *) malloc(len * sizeof(T));
            for (int i = 0; i < cur_len; ++i)
                new(new_data + i) T(data[i]);

            for (int i = 0;i < cur_len; ++i)//每一项的析构
                data[i].~T();
            free(data);
            data = new_data;
            max_size = len;
        }

        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert(iterator pos, const T &value) {
            //pos(iterator) - data(T*) is undefined!
            for (int i = cur_len - 1; i >= pos.p - data; --i) {
                data[i + 1] = data[i];
            }
            cur_len++;
            if (cur_len == max_size - 1) resize(max_size * 2);
            data[pos.p - data] = value;
            return iterator(pos);
        }

        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert(const size_t &ind, const T &value) {
            if (ind > max_size) throw index_out_of_bound();
            if (cur_len == max_size - 1) resize(max_size * 2);

            for (int i = cur_len - 1; i >= ind; --i) {
                data[i + 1] = data[i];
            }
            data[ind] = value;
            cur_len++;
            return iterator(ind, this);
        }

        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase(iterator pos) {

            (*(pos.p)).~T();
            for (int i = pos.p - data; i < cur_len - 1; ++i) {
                data[i] = data[i + 1];
            }
            cur_len--;
            return iterator(pos);
        }

        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const size_t &ind) {
            if (ind >= cur_len) throw index_out_of_bound();

            data[ind].~T();
            for (int i = ind; i < cur_len - 1; ++i) {
                data[i] = data[i + 1];
            }
            cur_len--;
            return iterator(ind, this);
        }

        void push_back(const T &value) {
            if (cur_len == max_size - 1) {
                resize(2 * max_size);
            }
            //此处尚未调用T的构造函数,所以要写成new,否则data[0]存储的就是乱码
            new (data + cur_len) T(value);
            cur_len++;
        }

        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back() {
            if (!cur_len) throw container_is_empty();
            else {
                data[cur_len - 1].~T();
                cur_len--;
            }
        }
    };


}

#endif

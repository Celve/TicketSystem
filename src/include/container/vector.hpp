#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "common/exceptions.hpp"

#include <climits>
#include <cstddef>

namespace thomas {

template<typename T>
class vector {
public:
    class iterator;
private:
    T *data;
    int cur_size;
    int max_size;

    void Clean() {
        for (int i = 0; i < cur_size; ++i)
            data[i].~T();
        free(data);
    }

    void DoubleSpace(iterator& pos) {
        max_size <<= 1;
        T *alternative = (T *)malloc(max_size * sizeof(T));
        for (int i = 0; i < cur_size; ++i) {
            new (alternative + i) T(data[i]);
            if (iterator(data + i, this) == pos)
                pos = iterator(alternative + i, this);
        }
        Clean();
        data = alternative;
    }

    int GetId(const iterator &it) {
        for (int i = 0; i < cur_size; ++i)
            if (iterator(data + i, this) == it)
                return i;
    }

public:
	/**
	 * TODO
	 * a type for actions of the elements of a vector, and you should write
	 *   a class named const_iterator with same interfaces.
	 */
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
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

	private:
         pointer ptr;
         const vector<T> *from;
	public:
		iterator(pointer ptr, const vector<T> *from):ptr(ptr), from(from) {}

		iterator operator+(const int &n) const {
            return iterator(ptr + n, from);
		}

		iterator operator-(const int &n) const {
            return iterator(ptr - n, from);
		}

		int operator-(const iterator &rhs) const {
            if (from != rhs.from)
                throw invalid_iterator();
            return ptr - rhs.ptr;
		}

		iterator& operator+=(const int &n) {
            ptr += n;
            return *this;
		}

		iterator& operator-=(const int &n) {
            ptr -= n;
            return *this;
		}

		iterator operator++(int) {
            iterator temp = *this;
            ++ptr;
            return temp;
        }

		iterator& operator++() {
            ++ptr;
            return *this;
        }

		iterator operator--(int) {
            iterator temp = *this;
            --ptr;
            return temp;
        }

		iterator& operator--() {
            --ptr;
            return *this;
        }

		T& operator*() const{
            return *ptr;
        }
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory address).
		 */
		bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }

		bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr;
        }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }

		bool operator!=(const const_iterator &rhs) const {
            return ptr != rhs.ptr;
        }
	};
	/**
	 * TODO
	 * has same function as iterator, just for a const object.
	 */
	class const_iterator {
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;
	
	private:
         pointer ptr;
         const vector<T> *from;

    public:
        const_iterator(pointer ptr, const vector<T> *from):ptr(ptr), from(from) {}

        const_iterator operator+(const int &n) const {
            return const_iterator(ptr + n, from);
        }

        const_iterator operator-(const int &n) const {
            return const_iterator(ptr - n, from);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const const_iterator &rhs) const {
            if (from != rhs.from)
                throw invalid_iterator();
            return ptr - rhs.ptr;
        }

        const_iterator& operator+=(const int &n) {
            ptr += n;
            return *this;
        }

        const_iterator& operator-=(const int &n) {
            ptr -= n;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++ptr;
            return temp;
        }

        const_iterator& operator++() {
            ++ptr;
            return *this;
        }

        const_iterator operator--(int) {
            iterator temp = *this;
            --ptr;
            return temp;
        }

        const_iterator& operator--() {
            --ptr;
            return *this;
        }

        T& operator*() const{
            return *ptr;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory address).
         */
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }

        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }

        bool operator!=(const const_iterator &rhs) const {
            return ptr != rhs.ptr;
        }
    };
	/**
	 * TODO Constructs
	 * Atleast two: default constructor, copy constructor
	 */
	vector() {
        cur_size = 0;
        max_size = 32;
        data = (T *)malloc(max_size * sizeof(T));
    }
	vector(const vector &other):cur_size(other.cur_size), max_size(other.max_size) {
        data = (T *)malloc(max_size * sizeof(T));
        for (int i = 0; i < cur_size; ++i)
            new (data + i) T(other.data[i]);
    }
	/**
	 * TODO Destructor
	 */
	~vector() {
        Clean();
    }

	vector &operator=(const vector &other) {
        if (this == &other)
            return *this;
        Clean();
        cur_size = other.cur_size;
        max_size = other.max_size;
        data = (T *)malloc(max_size * sizeof(T));
        for (int i = 0; i < cur_size; ++i)
            new (data + i) T(other.data[i]);
        return *this;
    }
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	T & at(const size_t &pos) {
        if (pos < 0 || pos >= cur_size)
            throw index_out_of_bound();
        return data[pos];
    }
	const T & at(const size_t &pos) const {
        if (pos < 0 || pos >= cur_size)
            throw index_out_of_bound();
        return data[pos];
    }
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	T & operator[](const size_t &pos) {
        return at(pos);
    }

	const T & operator[](const size_t &pos) const {
        return at(pos);
    }
	/**
	 * access the first element.
	 * throw container_is_empty if size == 0
	 */
	const T & front() const {
        if (!cur_size)
            throw container_is_empty();
        return data[0];
    }
	/**
	 * access the last element.
	 * throw container_is_empty if size == 0
	 */
	const T & back() const {
        if (!cur_size)
            throw container_is_empty();
        return data[cur_size - 1];
    }
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
        return iterator(data, this);
    }
	const_iterator cbegin() const {
        return const_iterator(data, this);
    }
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
        return iterator(data + cur_size, this);
    }
	const_iterator cend() const {
        return const_iterator(data + cur_size, this);
    }
	/**
	 * checks whether the container is empty
	 */
	bool empty() const {
        return !cur_size;
    }
	/**
	 * returns the number of elements
	 */
	size_t size() const {
        return cur_size;
    }
	/**
	 * clears the contents
	 */
	void clear() {
        cur_size = 0;
        max_size = 32;
        Clean();
        data = (T *)malloc(max_size * sizeof(T));
    }
	/**
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value.
	 */
	iterator insert(iterator pos, const T &value) {
        if (cur_size + 1 > max_size)
            DoubleSpace(pos);
        ++cur_size;
        int id = GetId(pos);
        for (int i = cur_size - 1; i >= id + 1; --i)
            new (data + i) T(data[i - 1]);
        new (data + id) T(value);
        return pos;
    }
	/**
	 * inserts value at index ind.
	 * after inserting, this->at(ind) == value
	 * returns an iterator pointing to the inserted value.
	 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
	 */
	iterator insert(const size_t &ind, const T &value) {
        if (ind > cur_size)
            throw index_out_of_bound();
        return insert(iterator(data + ind - 1, this), value);
    }
	/**
	 * removes the element at pos.
	 * return an iterator pointing to the following element.
	 * If the iterator pos refers the last element, the end() iterator is returned.
	 */
	iterator erase(iterator pos) {
        int id = GetId(pos);
        data[id].~T();
        for (int i = id; i < cur_size - 1; ++i)
            new (data + i) T(data[i + 1]);
        --cur_size;
        return pos;
    }
	/**
	 * removes the element with index ind.
	 * return an iterator pointing to the following element.
	 * throw index_out_of_bound if ind >= size
	 */
	iterator erase(const size_t &ind) {
        if (ind >= cur_size)
            throw index_out_of_bound();
        return erase(iterator(data + ind - 1, this));
    }
	/**
	 * adds an element to the end.
	 */
	void push_back(const T &value) {
        if (cur_size + 1 > max_size) {
            iterator temp(begin());
            DoubleSpace(temp);
        }
        new (data + cur_size) T(value);
        ++cur_size;
    }
	/**
	 * remove the last element from the end.
	 * throw container_is_empty if size() == 0
	 */
	void pop_back() {
        if (!cur_size)
            throw container_is_empty();
        data[cur_size - 1].~T();
        --cur_size;
    }
};


}

#endif

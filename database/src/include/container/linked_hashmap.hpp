//
// Created by Celve on 2022/4/10.
//

#ifndef LINKED_HASHMAP_LINKED_HASHMAP_H
#define LINKED_HASHMAP_LINKED_HASHMAP_H

// only for std::equal_to<T> and std::hash<T>
#include <cstddef>
#include <functional>

#include "common/exceptions.hpp"
// #include "container/pair.hpp"

namespace thomas {
template <class Key, class Value, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class linked_hashmap {
 public:
  // using value_type = pair<const Key, Value>;
  typedef std::pair<const Key, Value> value_type;  // NOLINT

 private:
  /* a set of fundamental functions */
  Hash *hash;
  Equal *comp;

  /* if load factor * capacity < current size, then expand the space */
  const size_t LOAD_FACTOR = 1;
  const size_t INIT_SIZE = 8192;

  /* the sum of entry in the hash table */
  size_t capacity;

  /* the sum of node in the hash table */
  size_t sum;

  /*
   * Node - Node in both hash table and link list.
   */
  class Node {
   private:
    /* the value_tyep inserted in the list */

    value_type *packet;

    /* the link list in hash map*/
    Node *prev_hash, *next_hash;
    Node *prev_list, *next_list;

    /* linked_hashmap can access*/
    friend class linked_hashmap;

   public:
    /*
     * init - Just init the pointer, the value_type is not init.
     */
    Node() {
      prev_hash = next_hash = nullptr;
      prev_list = next_list = nullptr;
      packet = nullptr;
    }

    /*
     * init - Init both.
     */
    explicit Node(const value_type &packet) {
      this->packet = new value_type(packet);
      prev_hash = next_hash = nullptr;
      prev_list = next_list = nullptr;
    }

    ~Node() { delete packet; }

    /*
     * InsertAfterHash - Insert a node in the hash table, after a particular node.
     */
    void InsertAfterHash(Node *const &prev) {
      Node *next = prev->next_hash;
      if (next) {
        next->prev_hash = this;
      }
      this->next_hash = next;
      prev->next_hash = this;
      this->prev_hash = prev;
    }

    /*
     * InsertAfterLink - Insert a node in the link list, after a particular node.
     */
    void InsertBeforeLink(Node *const &next) {
      Node *prev = next->prev_list;
      if (prev) {
        prev->next_list = this;
      }
      this->prev_list = prev;
      next->prev_list = this;
      this->next_list = next;
    }

    /*
     * Delete - Delete the node in both table and list.
     */
    void Delete() {
      if (prev_list) {
        prev_list->next_list = next_list;
      }
      if (next_list) {
        next_list->prev_list = prev_list;
      }
      if (prev_hash) {
        prev_hash->next_hash = next_hash;
      }
      if (next_hash) {
        next_hash->prev_hash = prev_hash;
      }
    }
  };

  /* the pointer array for the hash table */
  Node *table;

  /* the head pointer and tail pointer for list */
  Node *head, *tail;

  /*
   * Find - Find the node in the table for a specified key
   */
  Node *Find(const Key &key) const {
    size_t hash_value = (*hash)(key) % capacity;
    Node *curr = table[hash_value].next_hash;
    while (curr) {
      if ((*comp)(curr->packet->first, key)) {
        return curr;
      }
      curr = curr->next_hash;
    }
    return nullptr;
  }

  /*
   * Insert - Insert a non-exist value_type into the hash table.
   */
  Node *Insert(const value_type &packet) {
    size_t hash_value = (*hash)(packet.first) % capacity;
    Node *curr = new Node(packet);

    curr->InsertAfterHash(&table[hash_value]);
    curr->InsertBeforeLink(tail);

    return curr;
  }

  void Debug() {
    Node *curr = head->next_list;
    int m = 0;
    while (curr != tail) {
      curr = curr->next_list;
      ++m;
    }
    printf("%d\n", m);
    for (int i = 0; i < capacity; ++i) {
      int n = 0;
      Node *curr = &table[i];
      while (curr) {
        ++n;
        curr = curr->next_hash;
      }
      printf("%d ", n);
    }
    puts("");
  }

  /*
   * Expand - Expand the size of the table when too many elements contained.
   */
  void Expand() {
    if (sum <= capacity * LOAD_FACTOR) {
      return;
    }
    capacity <<= 1;
    Node *temp = new Node[capacity]();

    /* iterate and fill */
    Node *curr = head->next_list;
    while (curr != tail) {
      curr->prev_hash = curr->next_hash = nullptr;
      size_t hash_value = (*hash)(curr->packet->first) % capacity;
      curr->InsertAfterHash(&temp[hash_value]);
      curr = curr->next_list;
    }

    delete[] table;
    table = temp;
  }

  /*
   * Shrink - Shrink the size of the table when too less contained.
   */
  void Shrink() {
    if ((sum << 2) >= capacity || sum <= INIT_SIZE) {
      return;
    }
    capacity >>= 1;
    Node *temp = new Node[capacity]();

    /* iterate and fill */
    Node *curr = head->next_list;
    while (curr != tail) {
      curr->prev_hash = curr->next_hash = nullptr;
      size_t hash_value = (*hash)(curr->packet->first) % capacity;
      curr->InsertAfterHash(&temp[hash_value]);
      curr = curr->next_list;
    }

    delete[] table;
    table = temp;
  }

  /*
   * Clear
   */
  void Clear() {
    /* delete all elements */
    Node *curr = head->next_list;
    while (curr != tail) {
      Node *next = curr->next_list;
      delete curr;
      curr = next;
    }

    /* delete hash table */
    delete[] table;

    /* head and tail */
    head->next_list = tail;
    tail->prev_list = head;
  }

 public:
  class const_iterator;

  class iterator {
   private:
    Node *curr;
    const linked_hashmap *source;
    friend class linked_hashmap;

   public:
    using difference_type = std::ptrdiff_t;
    using value_type = Value;
    using pointer = Value *;
    using reference = Value &;
    using iterator_category = std::output_iterator_tag;

    iterator() {
      /* basic initiation */
      curr = nullptr;
      source = nullptr;
    }

    iterator(const iterator &other) {
      this->curr = other.curr;
      this->source = other.source;
    }

    iterator(Node *curr, const linked_hashmap *source) : curr(curr), source(source) {}

    /*
     * ++iter
     */
    iterator &operator++() {
      if (curr == source->tail) {
        throw invalid_iterator();
      }
      curr = curr->next_list;
      return *this;
    }

    /*
     * iter++
     */
    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    /*
     * --iter
     */
    iterator &operator--() {
      curr = curr->prev_list;
      if (curr == source->head) {
        throw invalid_iterator();
      }
      return *this;
    }

    /*
     * iter--
     */
    iterator operator--(int) {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    /*
     * *iter - Dereference.
     */
    linked_hashmap::value_type &operator*() const { return *curr->packet; }

    linked_hashmap::value_type *operator->() noexcept { return curr->packet; }

    bool operator==(const iterator &rhs) const { return curr == rhs.curr && source == rhs.source; }

    bool operator==(const const_iterator &rhs) const { return curr == rhs.curr && source == rhs.source; }

    bool operator!=(const iterator &rhs) const { return curr != rhs.curr || source != rhs.source; }

    bool operator!=(const const_iterator &rhs) const { return curr != rhs.curr || source != rhs.source; }
  };

  class const_iterator {
   private:
    const Node *curr;
    const linked_hashmap *source;
    friend class linked_hashmap;

   public:
    using difference_type = std::ptrdiff_t;
    using value_type = Value;
    using pointer = Value *;
    using reference = Value &;
    using iterator_category = std::output_iterator_tag;

    const_iterator() {
      /* basic initiation */
      curr = nullptr;
      source = nullptr;
    }

    const_iterator(const const_iterator &other) {
      this->curr = other.curr;
      this->source = other.source;
    }

    explicit const_iterator(const iterator &other) {
      this->curr = other.curr;
      this->source = other.source;
    }

    const_iterator(Node *curr, const linked_hashmap *source) : curr(curr), source(source) {}

    /*
     * ++iter
     */
    const_iterator &operator++() {
      if (curr == source->tail) {
        throw invalid_iterator();
      }
      curr = curr->next_list;
      return *this;
    }

    /*
     * iter++
     */
    const_iterator operator++(int) {
      const_iterator temp = *this;
      ++(*this);
      return temp;
    }

    /*
     * --iter
     */
    const_iterator &operator--() {
      curr = curr->prev_list;
      if (curr == source->head) {
        throw invalid_iterator();
      }
      return *this;
    }

    /*
     * iter--
     */
    const_iterator operator--(int) {
      const_iterator temp = *this;
      --(*this);
      return temp;
    }

    /*
     * *iter - Dereference.
     */
    const linked_hashmap::value_type &operator*() const { return *curr->packet; }

    linked_hashmap::value_type *operator->() const noexcept { return curr->packet; }

    bool operator==(const iterator &rhs) const { return curr == rhs.curr && source == rhs.source; }

    bool operator==(const const_iterator &rhs) const { return curr == rhs.curr && source == rhs.source; }

    bool operator!=(const iterator &rhs) const { return curr != rhs.curr || source != rhs.source; }

    bool operator!=(const const_iterator &rhs) const { return curr != rhs.curr || source != rhs.source; }
  };

  linked_hashmap() {
    /* set the initial capacity to fix the page size without considering the alignment*/
    capacity = INIT_SIZE;

    /* create a table */
    table = new Node[capacity]();

    /* create head and tail */
    head = new Node();
    tail = new Node();
    head->next_list = tail;
    tail->prev_list = head;

    /* basic initiation */
    hash = new Hash;
    comp = new Equal;
    sum = 0;
  }

  linked_hashmap(const linked_hashmap &other) {
    /* basic copy */
    capacity = other.capacity;
    sum = other.sum;

    /* basic new */
    head = new Node();
    tail = new Node();
    hash = new Hash;
    comp = new Equal;
    head->next_list = tail;
    tail->prev_list = head;
    table = new Node[capacity]();

    /* copy elements */
    Node *curr = other.head->next_list;
    while (curr != other.tail) {
      Insert(*curr->packet);
      curr = curr->next_list;
    }
  }

  linked_hashmap &operator=(const linked_hashmap &other) {
    if (&other == this) {
      return *this;
    }
    Clear();

    /* basic copy */
    capacity = other.capacity;
    sum = other.sum;

    /* allocation */
    table = new Node[capacity]();

    /* assignment */
    Node *curr = other.head->next_list;
    while (curr != other.tail) {
      Insert(*curr->packet);
      curr = curr->next_list;
    }
    return *this;
  }

  ~linked_hashmap() {
    Clear();
    delete hash;
    delete comp;
    delete head;
    delete tail;
  }

  /*
   * at - Use the key to get the value; Throw index_out_of_bound() when no such element exist.
   */
  Value &at(const Key &key) {
    Node *curr = Find(key);
    if (!curr) {
      throw index_out_of_bound();
    }
    return curr->packet->second;
  }

  const Value &at(const Key &key) const {
    Node *curr = Find(key);
    if (!curr) {
      throw index_out_of_bound();
    }
    return curr->packet->second;
  }

  /*
   * operator[] - Return the value of the key, create a new one if no exist
   */
  Value &operator[](const Key &key) {
    Node *curr = Find(key);

    /* doesn't exist, insert a new one with default value */
    if (curr == nullptr) {
      curr = Insert(value_type(key, Value()));
      ++sum;
      Expand();
    }

    return curr->packet->second;
  }

  /*
   * const operator [] - Return the value of the key, throw index_out_of_bound() when no exist
   */
  const Value &operator[](const Key &key) const {
    Node *curr = Find(key);
    if (!curr) {
      throw index_out_of_bound();
    }
    return curr->packet->second;
  }

  /*
   * begin - the iterator to the head of the list.
   */
  iterator begin() const { return iterator(head->next_list, this); }

  /*
   * cbegin - the const iterator to the head of the list.
   */
  const_iterator cbegin() const { return const_iterator(head->next_list, this); }

  /*
   * end - the iterator to the tail of the list.
   */
  iterator end() { return iterator(tail, this); }

  /*
   * cend - the iterator to the tail of the list.
   */
  const_iterator cend() const { return const_iterator(tail, this); }

  /*
   * size - Return the number of elements.
   */
  size_t size() const { return sum; }

  /*
   * empty - Return whether the container is empty.
   */
  bool empty() const { return sum == 0U; }

  /*
   * clear - Clear the all elements.
   */
  void clear() {
    Clear();

    /* basic initiation */
    capacity = INIT_SIZE;
    sum = 0;

    /* create hash table */
    table = new Node[capacity]();
  }

  /*
   * insert - Insert a value_type, and return a pair(iterator, bool).
   */
  std::pair<iterator, bool> insert(const value_type &value) {
    Node *curr = Find(value.first);
    bool flag = false;
    if (!curr) {
      curr = Insert(value);
      flag = true;
      ++sum;
      Expand();
    }
    return std::pair<iterator, bool>(iterator(curr, this), flag);
  }

  /*
   * erase - Erase the element at iterator pos.
   */
  void erase(iterator pos) {
    if (pos.source != this || pos.curr == tail) {
      throw invalid_iterator();
    }
    Node *curr = pos.curr;
    curr->Delete();
    delete curr;
    --sum;
    Shrink();
  }

  /*
   * count - Return whether the container has the key.
   */
  size_t count(const Key &key) const {
    Node *curr = Find(key);
    return curr != nullptr;
  }

  /*
   * find
   */
  iterator find(const Key &key) {
    Node *curr = Find(key);
    if (curr == nullptr) {
      curr = tail;
    }
    return iterator(curr, this);
  }

  /*
   * find
   */
  const_iterator find(const Key &key) const {
    Node *curr = Find(key);
    if (curr == nullptr) {
      curr = tail;
    }
    return const_iterator(curr, this);
  }
};
}  // namespace thomas

#endif  // LINKED_HASHMAP_LINKED_HASHMAP_H
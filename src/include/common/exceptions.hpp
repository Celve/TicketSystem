#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace thomas {

class linked_hashmap_error : public std::exception {};

class index_out_of_bound : public linked_hashmap_error {};

class runtime_error : public linked_hashmap_error {};

class invalid_iterator : public linked_hashmap_error {};

class container_is_empty : public linked_hashmap_error {};
}  // namespace thomas

#endif
#pragma once

#include "type/dual_string.h"
#include "type/string_int.h"

namespace thomas {

#define DISALLOW_COPY(class_name)          \
  class_name(const class_name &) = delete; \
  class_name operator=(const class_name &) = delete;  // NOLINT

#define DECLARE(class_name)                                                            \
  template class class_name<String<48>, size_t, StringComparator<48>>;    /* NOLINT */ \
  template class class_name<StringInt<68>, int, StringIntComparator<68>>; /* NOLINT */

#define DECLARE_BPLUSTREE_INTERNALPAGE(class_name)                                           \
  template class class_name<String<48>, page_id_t, StringComparator<48>>;       /* NOLINT */ \
  template class class_name<StringInt<68>, page_id_t, StringIntComparator<68>>; /* NOLINT */

}  // namespace thomas

#pragma once

#include "Account.h"
#include "TrainSystem.h"
#include "type/dual_string.h"
#include "type/string_any.h"

namespace thomas {

#define DISALLOW_COPY(class_name)          \
  class_name(const class_name &) = delete; \
  class_name operator=(const class_name &) = delete;  // NOLINT

#define DECLARE(class_name)                                                                           \
  template class class_name<String<48>, size_t, StringComparator<48>>;                   /* NOLINT */ \
  template class class_name<StringAny<68, int>, int, StringAnyComparator<68, int>>;      /* NOLINT */ \
  template class class_name<String<32>, User, StringComparator<32>>;                     /* NOLINT */ \
  template class class_name<DualString<32, 32>, Station, DualStringComparator<32, 32>>;  /* NOLINT */ \
  template class class_name<StringAny<32, int>, DayTrain, StringAnyComparator<32, int>>; /* NOLINT */ \
  template class class_name<StringAny<32, int>, Order, StringAnyComparator<32, int>>;    /* NOLINT */

#define DECLARE_BPLUSTREE_INTERNALPAGE(class_name)                                                     \
  template class class_name<String<32>, page_id_t, StringComparator<32>>;                 /* NOLINT */ \
  template class class_name<String<48>, page_id_t, StringComparator<48>>;                 /* NOLINT */ \
  template class class_name<DualString<32, 32>, page_id_t, DualStringComparator<32, 32>>; /* NOLINT */ \
  template class class_name<StringAny<32, int>, page_id_t, StringAnyComparator<32, int>>; /* NOLINT */ \
  template class class_name<StringAny<68, int>, page_id_t, StringAnyComparator<68, int>>; /* NOLINT */

}  // namespace thomas

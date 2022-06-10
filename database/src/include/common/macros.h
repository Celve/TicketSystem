#pragma once

#include "Account.h"
#include "TrainSystem.h"
#include "type/dual_string.h"
#include "type/string_type.h"

namespace thomas {

#define DISALLOW_COPY(class_name)          \
  class_name(const class_name &) = delete; \
  class_name operator=(const class_name &) = delete;  // NOLINT

#define DECLARE(class_name)                                                                      \
  template class class_name<String<48>, size_t, StringComparator<48>>;              /* NOLINT */ \
  template class class_name<StringAny<int, 68>, int, StringAnyComparator<68>>;      /* NOLINT */ \
  template class class_name<String<32>, User, StringComparator<32>>;                /* NOLINT */ \
  template class class_name<DualString<32>, Station, DualStringComparator<32>>;     /* NOLINT */ \
  template class class_name<StringAny<int, 32>, DayTrain, StringAnyComparator<32>>; /* NOLINT */ \
  template class class_name<StringAny<int, 32>, Order, StringAnyComparator<32>>;    /* NOLINT */

#define DECLARE_BPLUSTREE_INTERNALPAGE(class_name)                                                \
  template class class_name<String<32>, page_id_t, StringComparator<32>>;            /* NOLINT */ \
  template class class_name<String<48>, page_id_t, StringComparator<48>>;            /* NOLINT */ \
  template class class_name<StringAny<int, 32>, page_id_t, StringAnyComparator<32>>; /* NOLINT */ \
  template class class_name<StringAny<int, 68>, page_id_t, StringAnyComparator<68>>; /* NOLINT */

}  // namespace thomas

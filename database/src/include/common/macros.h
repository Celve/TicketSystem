#pragma once

#include "Account.h"
#include "TrainSystem.h"
#include "type/dual_string.h"
#include "type/string_any.h"
#include "type/string_int_int.h"

namespace thomas {

#define DISALLOW_COPY(class_name)          \
  class_name(const class_name &) = delete; \
  class_name operator=(const class_name &) = delete;  // NOLINT

#define DECLARE(class_name)                                                                           \
  template class class_name<String<24>, size_t, StringComparator<24>>;                   /* NOLINT */ \
  template class class_name<String<32>, size_t, StringComparator<32>>;                   /* NOLINT */ \
  template class class_name<String<48>, size_t, StringComparator<48>>;                   /* NOLINT */ \
  template class class_name<StringAny<68, int>, int, StringAnyComparator<68, int>>;      /* NOLINT */ \
  template class class_name<String<24>, User, StringComparator<24>>;                     /* NOLINT */ \
  template class class_name<String<32>, User, StringComparator<32>>;                     /* NOLINT */ \
  template class class_name<String<24>, Train, StringComparator<24>>;                    /* NOLINT */ \
  template class class_name<String<32>, Train, StringComparator<32>>;                    /* NOLINT */ \
  template class class_name<DualString<32, 24>, Station, DualStringComparator<32, 24>>;  /* NOLINT */ \
  template class class_name<DualString<32, 32>, Station, DualStringComparator<32, 32>>;  /* NOLINT */ \
  template class class_name<StringAny<24, int>, DayTrain, StringAnyComparator<24, int>>; /* NOLINT */ \
  template class class_name<StringAny<32, int>, DayTrain, StringAnyComparator<32, int>>; /* NOLINT */ \
  template class class_name<StringAny<24, int>, Order, StringAnyComparator<24, int>>;    /* NOLINT */ \
  template class class_name<StringAny<32, int>, Order, StringAnyComparator<32, int>>;    /* NOLINT */ \
  template class class_name<StringIntInt<24>, PendingOrder, StringIntIntComparator<24>>; /* NOLINT */ \
  template class class_name<StringIntInt<32>, PendingOrder, StringIntIntComparator<32>>; /* NOLINT */

#define DECLARE_BPLUSTREE_INTERNALPAGE(class_name)                                                     \
  template class class_name<String<24>, page_id_t, StringComparator<24>>;                 /* NOLINT */ \
  template class class_name<String<32>, page_id_t, StringComparator<32>>;                 /* NOLINT */ \
  template class class_name<String<48>, page_id_t, StringComparator<48>>;                 /* NOLINT */ \
  template class class_name<DualString<32, 24>, page_id_t, DualStringComparator<32, 24>>; /* NOLINT */ \
  template class class_name<DualString<32, 32>, page_id_t, DualStringComparator<32, 32>>; /* NOLINT */ \
  template class class_name<StringAny<24, int>, page_id_t, StringAnyComparator<24, int>>; /* NOLINT */ \
  template class class_name<StringAny<32, int>, page_id_t, StringAnyComparator<32, int>>; /* NOLINT */ \
  template class class_name<StringAny<68, int>, page_id_t, StringAnyComparator<68, int>>; /* NOLINT */ \
  template class class_name<StringIntInt<24>, page_id_t, StringIntIntComparator<24>>;     /* NOLINT */ \
  template class class_name<StringIntInt<32>, page_id_t, StringIntIntComparator<32>>;     /* NOLINT */

}  // namespace thomas

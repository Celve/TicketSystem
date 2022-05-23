#include "type/mixed_string_int.h"

namespace thomas {
  bool MixedStringIntForStringComparator(const MixedStringInt<68> &lhs, const MixedStringInt<68> &rhs) {
    return lhs.CompareStringWith(rhs);
  }
}
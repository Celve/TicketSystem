#include "type/mixed_string_int.h"

namespace thomas {
  bool MixedStringIntForStringComparator(const MixedStringInt<64> &lhs, const MixedStringInt<64> &rhs) {
    return lhs.CompareStringWith(rhs);
  }
}
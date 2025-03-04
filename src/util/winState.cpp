#include "winState.h"

#include <cstdint>

WinState operator-(const WinState original) {
  return WinState(-(int8_t)original);
}

bool operator<(const WinState lhs, const WinState rhs) {
  return (int8_t)lhs < (int8_t)rhs;
}

#pragma once

#include <cstdint>
#include <memory>
#include <ostream>

#include "color.h"

struct Offset {
  int8_t dx;
  int8_t dy;

  Offset Orient(const Color player) const;

  bool operator==(const Offset& other) const;
  Offset operator-() const;
};

template <>
struct std::hash<Offset> {
  size_t operator()(const Offset& offset) const noexcept {
    return ((int16_t)offset.dx) << 8 + offset.dy;
  }
};

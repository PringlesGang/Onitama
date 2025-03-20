#pragma once

#include <cstdint>
#include <memory>
#include <ostream>

#include "color.h"

struct Offset {
  int dx;
  int dy;

  Offset Orient(const Color player) const;

  bool operator==(const Offset& other) const;
  Offset operator-() const;
};

template <>
struct std::hash<Offset> {
  size_t operator()(const Offset& offset) const noexcept {
    return (((int16_t)offset.dx) << (8 * sizeof(int8_t))) + offset.dy;
  }
};

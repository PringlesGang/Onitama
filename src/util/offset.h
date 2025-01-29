#pragma once

#include <cstdint>
#include <memory>
#include <ostream>

struct Offset {
  int8_t dx;
  int8_t dy;

  bool operator==(const Offset& other) const;
  Offset operator-() const;
};

template <>
struct std::hash<Offset> {
  size_t operator()(const Offset& offset) const noexcept {
    return ((int16_t)offset.dx) << 8 + offset.dy;
  }
};

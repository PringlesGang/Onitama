#pragma once

#include <optional>

#include "offset.h"

struct Coordinate {
  Coordinate(const size_t x, const size_t y);

  Coordinate operator+(const Coordinate other) const;
  void operator+=(const Coordinate other);
  bool operator==(const Coordinate other) const;
  void operator=(const Coordinate other);

  std::optional<Coordinate> try_add(const Offset direction) const;

  size_t x;
  size_t y;
};

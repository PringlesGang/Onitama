#pragma once

#include <optional>

#include "offset.h"

struct Coordinate {
  Coordinate(size_t x, size_t y);

  Coordinate operator+(Coordinate other) const;
  void operator+=(Coordinate other);
  bool operator==(Coordinate other) const;
  void operator=(Coordinate other);

  std::optional<Coordinate> try_add(Offset direction) const;

  size_t x;
  size_t y;
};

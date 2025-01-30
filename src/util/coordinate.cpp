#include "coordinate.h"

Coordinate::Coordinate(const size_t x, const size_t y) : x(x), y(y) {}

Coordinate Coordinate::operator+(const Coordinate other) const {
  return Coordinate{this->x + other.x, this->y + other.y};
}

void Coordinate::operator+=(const Coordinate other) {
  this->x += other.x;
  this->y += other.y;
}

bool Coordinate::operator==(const Coordinate other) const {
  return this->x == other.x && this->y == other.y;
}

void Coordinate::operator=(const Coordinate other) {
  this->x = other.x;
  this->y = other.y;
}

std::optional<Coordinate> Coordinate::try_add(const Offset direction) const {
  if (direction.dx < 0 && this->x < (size_t)-direction.dx ||
      direction.dy < 0 && this->y < (size_t)-direction.dy)
    return std::nullopt;

  return Coordinate{this->x + direction.dx, this->y + direction.dy};
}

#include "coordinate.h"

Coordinate::Coordinate(size_t x, size_t y) {
  this->x = x;
  this->y = y;
}

Coordinate Coordinate::operator+(Coordinate other) const {
  return Coordinate(this->x + other.x, this->y + other.y);
}

void Coordinate::operator+=(Coordinate other) {
  this->x += other.x;
  this->y += other.y;
}

bool Coordinate::operator==(Coordinate other) const {
  return this->x == other.x && this->y == other.y;
}

void Coordinate::operator=(Coordinate other) {
  this->x = other.x;
  this->y = other.y;
}

std::optional<Coordinate> Coordinate::try_add(Offset direction) {
  if (this->x < direction.dx || this->y < direction.dy)
    return std::optional<Coordinate>();

  return std::optional<Coordinate>(
      Coordinate(this->x + direction.dx, this->y + direction.dy));
}

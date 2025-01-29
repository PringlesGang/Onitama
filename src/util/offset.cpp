#include "offset.h"

bool Offset::operator==(const Offset& other) const {
  return dx == other.dx && dy == other.dy;
}

Offset Offset::operator-() const {
  return Offset{.dx = (int8_t)-dx, .dy = (int8_t)-dy};
}

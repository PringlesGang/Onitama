#include "offset.h"

#include "../constants.h"
#include "../game/board.h"

Offset Offset::Orient(const Color player) const {
  return player == TopPlayer ? -*this : *this;
}

bool Offset::operator==(const Offset& other) const {
  return dx == other.dx && dy == other.dy;
}

Offset Offset::operator-() const {
  return Offset{.dx = (int8_t)-dx, .dy = (int8_t)-dy};
}

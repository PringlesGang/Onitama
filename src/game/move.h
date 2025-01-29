#pragma once

#include "../util/coordinate.h"
#include "../util/offset.h"
#include "card.h"

namespace Game {

struct Move {
  Coordinate Source;
  Offset OrientedOffset;
  Card UsedCard;

  bool operator==(const Move& move) const;
};

}  // namespace Game

template <>
struct std::hash<Game::Move> {
  size_t operator()(const Game::Move& move) const noexcept {
    return ((move.Source.x + move.OrientedOffset.dx) << 16) ^
           ((move.Source.y + move.OrientedOffset.dy) << 8) ^
           (size_t)move.UsedCard.Type;
  }
};

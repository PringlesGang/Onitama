#pragma once

#include "../util/coordinate.h"
#include "../util/offset.h"
#include "card.h"

namespace Game {

struct Move {
  size_t PawnId;
  Card UsedCard;
  size_t OffsetId;

  bool operator==(const Move& move) const;
};

}  // namespace Game

template <>
struct std::hash<Game::Move> {
  size_t operator()(const Game::Move& move) const noexcept {
    return (move.PawnId << 16) ^ (move.OffsetId << 8) ^
           (size_t)move.UsedCard.Type;
  }
};

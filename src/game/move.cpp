#include "move.h"

namespace Game {

bool Move::operator==(const Move& other) const {
  return PawnId == other.PawnId && OffsetId == other.OffsetId &&
         UsedCard == other.UsedCard;
}

}  // namespace Game

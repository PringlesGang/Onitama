#include "move.h"

namespace Game {

bool Move::operator==(const Move& other) const {
  return Source == other.Source && OrientedOffset == other.OrientedOffset &&
         UsedCard == other.UsedCard;
}

}  // namespace Game

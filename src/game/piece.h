#pragma once

#include "../util/color.h"

namespace Game {

struct Piece {
  bool operator==(const Piece& other) const {
    return Team == other.Team && Master == other.Master;
  }

  Color Team;
  bool Master;
};

}  // namespace Game

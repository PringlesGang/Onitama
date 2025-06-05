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

inline std::string to_string(Game::Piece piece) {
  std::string string = to_string(piece.Team);
  return string + (piece.Master ? " master" : " student");
}

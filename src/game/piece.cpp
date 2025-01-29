#include "piece.h"

#include <utility>

namespace Game {

Piece::Piece(Color color, bool master) : Team(color), Master(master) {};
Piece::Piece(const Piece& piece) : Team(piece.Team), Master(piece.Master) {};
Piece::Piece(Piece&& piece)
    : Team(std::move(piece.Team)), Master(std::move(piece.Master)) {};

void Piece::operator=(const Piece& other) {
  Master = other.Master;
  Team = other.Team;
}

}  // namespace Game

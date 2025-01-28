#include "piece.h"

#include <utility>

Color operator~(const Color& orig) {
  return orig == Color::Red ? Color::Blue : Color::Red;
}

Piece::Piece(Color color, bool master) : Team(color), Master(master) {};
Piece::Piece(const Piece& piece) : Team(piece.Team), Master(piece.Master) {};
Piece::Piece(Piece&& piece)
    : Team(std::move(piece.Team)), Master(std::move(piece.Master)) {};

void Piece::operator=(const Piece& other) {
  Master = other.Master;
  Team = other.Team;
}

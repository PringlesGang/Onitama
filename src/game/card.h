#pragma once

#include <ostream>
#include <vector>

#include "../util/offset.h"
#include "piece.h"

namespace Game {

enum class CardType {
  Boar,
  Cobra,
  Crab,
  Crane,
  Dragon,
  Eel,
  Elephant,
  Frog,
  Goose,
  Horse,
  Mantis,
  Monkey,
  Ox,
  Rabbit,
  Rooster,
  Tiger,
  CardTypeCount,
};

struct Card {
  static std::vector<Offset> GetMoves(CardType card);
  static Color GetColor(CardType card);

  Color GetColor() const { return GetColor(Type); };
  std::vector<Offset> GetMoves() const { return GetMoves(Type); };
  bool HasMove(const Offset offset) const;

  bool operator==(const Card& other) const;

  CardType Type = CardType(0);

  std::ostream& StreamRow(std::ostream& stream, int8_t row,
                          const bool rotate = false) const;
  std::ostream& Stream(std::ostream& stream, const bool rotate = false) const;
};

}  // namespace Game

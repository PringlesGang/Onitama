#pragma once

#include <ostream>
#include <string>
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
  static std::vector<Offset> GetMoves(const CardType card);
  static Color GetColor(const CardType card);
  static std::string GetName(const CardType card);

  Color GetColor() const { return GetColor(Type); };
  std::vector<Offset> GetMoves() const { return GetMoves(Type); };
  std::string GetName() const { return GetName(Type); };
  bool HasMove(const Offset offset) const;

  bool operator==(const Card& other) const { return Type == other.Type; }

  CardType Type = CardType(0);

  std::ostream& StreamRow(std::ostream& stream, const int8_t row,
                          const bool rotate = false) const;
  std::ostream& Stream(std::ostream& stream, const bool rotate = false) const;
};

}  // namespace Game

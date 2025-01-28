#pragma once

#include <cstdint>
#include <vector>

#include "piece.h"

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

struct Offset {
  int8_t dx;
  int8_t dy;
};

struct Card {
  static std::vector<Offset> GetMoves(CardType card);
  static Color GetColor(CardType card);

  Color GetColor() const { return GetColor(Type); };
  std::vector<Offset> GetMoves() const { return GetMoves(Type); };

  CardType Type = CardType(0);
};

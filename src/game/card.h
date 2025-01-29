#pragma once

#include <ostream>
#include <unordered_set>

#include "../util/offset.h"
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

struct Card {
  static std::unordered_set<Offset> GetMoves(CardType card);
  static Color GetColor(CardType card);

  Color GetColor() const { return GetColor(Type); };
  std::unordered_set<Offset> GetMoves() const { return GetMoves(Type); };

  bool operator==(const Card& other) const;

  CardType Type = CardType(0);

  friend std::ostream& operator<<(std::ostream& stream, const Card& card);
};

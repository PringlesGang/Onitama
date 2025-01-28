#pragma once

#include <cstdint>
#include <unordered_set>

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

  bool operator==(const Offset& other) const;
};

template <>
struct std::hash<Offset> {
  size_t operator()(const Offset& offset) const noexcept {
    return ((int16_t)offset.dx) << 8 + offset.dy;
  }
};

struct Card {
  static std::unordered_set<Offset> GetMoves(CardType card);
  static Color GetColor(CardType card);

  Color GetColor() const { return GetColor(Type); };
  std::unordered_set<Offset> GetMoves() const { return GetMoves(Type); };

  bool operator==(const Card& other) const;

  CardType Type = CardType(0);
};

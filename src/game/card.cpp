#include "card.h"

#include <format>
#include <stdexcept>
#include <string>

std::unordered_set<Offset> Card::GetMoves(CardType card) {
  switch (card) {
    case CardType::Boar:
      return std::unordered_set({Offset(-1, 0), Offset(0, -1), Offset(1, 0)});
    case CardType::Cobra:
      return std::unordered_set({Offset(-1, 0), Offset(1, -1), Offset(1, 1)});
    case CardType::Crab:
      return std::unordered_set({Offset(-2, 0), Offset(0, -1), Offset(2, 0)});
    case CardType::Crane:
      return std::unordered_set({Offset(-1, 1), Offset(0, -1), Offset(1, 1)});
    case CardType::Dragon:
      return std::unordered_set(
          {Offset(-2, -1), Offset(-1, 1), Offset(1, 1), Offset(2, -1)});
    case CardType::Eel:
      return std::unordered_set({Offset(-1, -1), Offset(-1, 1), Offset(1, 0)});
    case CardType::Elephant:
      return std::unordered_set(
          {Offset(-1, -1), Offset(-1, 0), Offset(1, -1), Offset(1, 0)});
    case CardType::Frog:
      return std::unordered_set({Offset(-2, 0), Offset(-1, -1), Offset(1, 1)});
    case CardType::Goose:
      return std::unordered_set(
          {Offset(-1, -1), Offset(-1, 0), Offset(1, 0), Offset(1, 1)});
    case CardType::Horse:
      return std::unordered_set({Offset(-1, 0), Offset(0, -1), Offset(0, 1)});
    case CardType::Mantis:
      return std::unordered_set({Offset(-1, -1), Offset(0, 1), Offset(1, -1)});
    case CardType::Monkey:
      return std::unordered_set(
          {Offset(-1, -1), Offset(-1, 1), Offset(1, -1), Offset(1, 1)});
    case CardType::Ox:
      return std::unordered_set({Offset(0, -1), Offset(0, 1), Offset(1, 0)});
    case CardType::Rabbit:
      return std::unordered_set({Offset(-1, 1), Offset(1, -1), Offset(2, 0)});
    case CardType::Rooster:
      return std::unordered_set(
          {Offset(-1, 0), Offset(-1, 1), Offset(1, 0), Offset(1, -1)});
    case CardType::Tiger:
      return std::unordered_set({Offset(0, -2), Offset(0, 1)});

    default:
      size_t cardNum = (size_t)card;
      throw std::runtime_error(
          std::vformat("Invalid card type {}", std::make_format_args(cardNum)));
  }
}

Color Card::GetColor(CardType card) {
  switch (card) {
    case CardType::Boar:
    case CardType::Cobra:
    case CardType::Dragon:
    case CardType::Elephant:
    case CardType::Frog:
    case CardType::Horse:
    case CardType::Mantis:
    case CardType::Rooster:
      return Color::Red;

    case CardType::Crab:
    case CardType::Crane:
    case CardType::Eel:
    case CardType::Goose:
    case CardType::Monkey:
    case CardType::Ox:
    case CardType::Rabbit:
    case CardType::Tiger:
      return Color::Blue;

    default:
      size_t cardNum = (size_t)card;
      throw std::runtime_error(
          std::vformat("Invalid card type {}", std::make_format_args(cardNum)));
  }
}

bool Card::operator==(const Card& other) const { return Type == other.Type; }

std::ostream& Card::StreamRow(std::ostream& stream, int8_t row,
                              const bool rotate) const {
  const std::unordered_set<Offset> offsets = GetMoves();
  const int8_t sign = rotate ? -1 : 1;
  Offset offset{.dx = -2 * sign, .dy = row};

  for (; abs(offset.dx) <= 2; offset.dx += sign) {
    if (offsets.contains(offset)) {
      stream << 'X';
    } else {
      stream << ((offset.dx == 0 && row == 0) ? 'O' : '.');
    }
  }

  return stream;
}

std::ostream& Card::Stream(std::ostream& stream, const bool rotate) const {
  const std::unordered_set<Offset> offsets = GetMoves();
  const int8_t sign = rotate ? -1 : 1;
  Offset offset{.dx = -2 * sign, .dy = -2 * sign};

  stream << std::endl;
  for (; abs(offset.dy) <= 2; offset.dy += sign) {
    for (; abs(offset.dx) <= 2; offset.dx += sign) {
      if (offsets.contains(offset)) {
        stream << 'X';
      } else {
        stream << ((offset.dx == 0 && offset.dy == 0) ? 'O' : '.');
      }
    }

    stream << std::endl;
  }

  return stream;
}

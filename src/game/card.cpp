#include "card.h"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>

#include "../util/ansiColor.h"

namespace Game {

std::vector<Offset> Card::GetMoves(CardType card) {
  switch (card) {
    case CardType::Boar:
      return std::vector({Offset(-1, 0), Offset(0, -1), Offset(1, 0)});
    case CardType::Cobra:
      return std::vector({Offset(-1, 0), Offset(1, -1), Offset(1, 1)});
    case CardType::Crab:
      return std::vector({Offset(-2, 0), Offset(0, -1), Offset(2, 0)});
    case CardType::Crane:
      return std::vector({Offset(-1, 1), Offset(0, -1), Offset(1, 1)});
    case CardType::Dragon:
      return std::vector(
          {Offset(-2, -1), Offset(-1, 1), Offset(1, 1), Offset(2, -1)});
    case CardType::Eel:
      return std::vector({Offset(-1, -1), Offset(-1, 1), Offset(1, 0)});
    case CardType::Elephant:
      return std::vector(
          {Offset(-1, -1), Offset(-1, 0), Offset(1, -1), Offset(1, 0)});
    case CardType::Frog:
      return std::vector({Offset(-2, 0), Offset(-1, -1), Offset(1, 1)});
    case CardType::Goose:
      return std::vector(
          {Offset(-1, -1), Offset(-1, 0), Offset(1, 0), Offset(1, 1)});
    case CardType::Horse:
      return std::vector({Offset(-1, 0), Offset(0, -1), Offset(0, 1)});
    case CardType::Mantis:
      return std::vector({Offset(-1, -1), Offset(0, 1), Offset(1, -1)});
    case CardType::Monkey:
      return std::vector(
          {Offset(-1, -1), Offset(-1, 1), Offset(1, -1), Offset(1, 1)});
    case CardType::Ox:
      return std::vector({Offset(0, -1), Offset(0, 1), Offset(1, 0)});
    case CardType::Rabbit:
      return std::vector({Offset(-1, 1), Offset(1, -1), Offset(2, 0)});
    case CardType::Rooster:
      return std::vector(
          {Offset(-1, 0), Offset(-1, 1), Offset(1, 0), Offset(1, -1)});
    case CardType::Tiger:
      return std::vector({Offset(0, -2), Offset(0, 1)});

    default:
      const size_t cardNum = (size_t)card;
      throw std::runtime_error(std::format("Invalid card type {}", cardNum));
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
      const size_t cardNum = (size_t)card;
      throw std::runtime_error(std::format("Invalid card type {}", cardNum));
  }
}

std::string Card::GetName(CardType card) {
  switch (card) {
    case CardType::Boar:
      return "boar";
    case CardType::Cobra:
      return "cobra";
    case CardType::Crab:
      return "crab";
    case CardType::Crane:
      return "crane";
    case CardType::Dragon:
      return "dragon";
    case CardType::Eel:
      return "eel";
    case CardType::Elephant:
      return "elephant";
    case CardType::Frog:
      return "frog";
    case CardType::Goose:
      return "goose";
    case CardType::Horse:
      return "horse";
    case CardType::Mantis:
      return "mantis";
    case CardType::Monkey:
      return "monkey";
    case CardType::Ox:
      return "ox";
    case CardType::Rabbit:
      return "rabbit";
    case CardType::Rooster:
      return "rooster";
    case CardType::Tiger:
      return "tiger";

    default:
      const size_t cardNum = (size_t)card;
      throw std::runtime_error(std::format("Invalid card type {}", cardNum));
  }
}

bool Card::HasMove(const Offset offset) const {
  const std::vector moves = GetMoves();
  return std::find(moves.begin(), moves.end(), offset) != moves.end();
}

std::ostream& Card::StreamRow(std::ostream& stream, const int8_t row,
                              const bool rotate) const {
  const std::vector<Offset> offsets = GetMoves();
  const int8_t sign = rotate ? -1 : 1;

  for (Offset offset{.dx = -2 * sign, .dy = row}; abs(offset.dx) <= 2;
       offset.dx += sign) {
    const auto offsetId = std::find(offsets.begin(), offsets.end(), offset);

    if (offsetId != offsets.end()) {
      stream << AnsiColor::Color(AnsiColor::Foreground::White,
                                 AnsiColor::Background::Gray)
             << std::distance(offsets.begin(), offsetId) << AnsiColor::Reset();
    } else if (offset.dx == 0 && row == 0) {
      stream << AnsiColor::Invert() << ' ' << AnsiColor::Reset();
    } else {
      stream << '.';
    }
  }

  return stream;
}

std::ostream& Card::Stream(std::ostream& stream, const bool rotate) const {
  const int8_t sign = rotate ? -1 : 1;

  stream << std::endl;
  for (int8_t row = -2 * sign; abs(row) <= 2; row++) {
    StreamRow(stream, row, rotate) << std::endl;
  }

  return stream;
}

}  // namespace Game

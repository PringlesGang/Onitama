#include "card.h"

#include <format>
#include <stdexcept>
#include <string>

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

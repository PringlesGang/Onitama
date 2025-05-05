#pragma once

#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../util/offset.h"
#include "piece.h"

namespace Game {

constexpr size_t CARD_DISPLAY_SIZE = 5;

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

const static inline std::vector<Offset> BoarMoves{Offset{-1, 0}, Offset{0, -1},
                                                  Offset{1, 0}};
const static inline std::vector<Offset> CobraMoves{Offset{-1, 0}, Offset{1, -1},
                                                   Offset{1, 1}};
const static inline std::vector<Offset> CrabMoves{Offset{-2, 0}, Offset{0, -1},
                                                  Offset{2, 0}};
const static inline std::vector<Offset> CraneMoves{Offset{-1, 1}, Offset{0, -1},
                                                   Offset{1, 1}};
const static inline std::vector<Offset> DragonMoves{
    Offset{-2, -1}, Offset{-1, 1}, Offset{1, 1}, Offset{2, -1}};
const static inline std::vector<Offset> EelMoves{Offset{-1, -1}, Offset{-1, 1},
                                                 Offset{1, 0}};
const static inline std::vector<Offset> ElephantMoves{
    Offset{-1, -1}, Offset{-1, 0}, Offset{1, -1}, Offset{1, 0}};
const static inline std::vector<Offset> FrogMoves{Offset{-2, 0}, Offset{-1, -1},
                                                  Offset{1, 1}};
const static inline std::vector<Offset> GooseMoves{
    Offset{-1, -1}, Offset{-1, 0}, Offset{1, 0}, Offset{1, 1}};
const static inline std::vector<Offset> HorseMoves{Offset{-1, 0}, Offset{0, -1},
                                                   Offset{0, 1}};
const static inline std::vector<Offset> MantisMoves{
    Offset{-1, -1}, Offset{0, 1}, Offset{1, -1}};
const static inline std::vector<Offset> MonkeyMoves{
    Offset{-1, -1}, Offset{-1, 1}, Offset{1, -1}, Offset{1, 1}};
const static inline std::vector<Offset> OxMoves{Offset{0, -1}, Offset{0, 1},
                                                Offset{1, 0}};
const static inline std::vector<Offset> RabbitMoves{
    Offset{-1, 1}, Offset{1, -1}, Offset{2, 0}};
const static inline std::vector<Offset> RoosterMoves{
    Offset{-1, 0}, Offset{-1, 1}, Offset{1, 0}, Offset{1, -1}};
const static inline std::vector<Offset> TigerMoves{Offset{0, -2}, Offset{0, 1}};

const static inline std::unordered_map<CardType, const std::vector<Offset>&>
    CardToMoves{
        {CardType::Boar, BoarMoves},
        {CardType::Cobra, CobraMoves},
        {CardType::Crab, CrabMoves},
        {CardType::Crane, CraneMoves},
        {CardType::Dragon, DragonMoves},
        {CardType::Eel, EelMoves},
        {CardType::Elephant, ElephantMoves},
        {CardType::Frog, FrogMoves},
        {CardType::Goose, GooseMoves},
        {CardType::Horse, HorseMoves},
        {CardType::Mantis, MantisMoves},
        {CardType::Monkey, MonkeyMoves},
        {CardType::Ox, OxMoves},
        {CardType::Rabbit, RabbitMoves},
        {CardType::Rooster, RoosterMoves},
        {CardType::Tiger, TigerMoves},
    };

const static inline std::unordered_map<CardType, Color> CardToColor{
    {CardType::Boar, Color::Red},    {CardType::Cobra, Color::Red},
    {CardType::Dragon, Color::Red},  {CardType::Elephant, Color::Red},
    {CardType::Frog, Color::Red},    {CardType::Horse, Color::Red},
    {CardType::Mantis, Color::Red},  {CardType::Rooster, Color::Red},
    {CardType::Crab, Color::Blue},   {CardType::Crane, Color::Blue},
    {CardType::Eel, Color::Blue},    {CardType::Goose, Color::Blue},
    {CardType::Monkey, Color::Blue}, {CardType::Ox, Color::Blue},
    {CardType::Rabbit, Color::Blue}, {CardType::Tiger, Color::Blue},
};

const static inline std::unordered_map<CardType, std::string> CardToString{
    {CardType::Boar, "boar"},
    {CardType::Cobra, "cobra"},
    {CardType::Crab, "crab"},
    {CardType::Crane, "crane"},
    {CardType::Dragon, "dragon"},
    {CardType::Eel, "eel"},
    {CardType::Elephant, "elephant"},
    {CardType::Frog, "frog"},
    {CardType::Goose, "goose"},
    {CardType::Horse, "horse"},
    {CardType::Mantis, "mantis"},
    {CardType::Monkey, "monkey"},
    {CardType::Ox, "ox"},
    {CardType::Rabbit, "rabbit"},
    {CardType::Rooster, "rooster"},
    {CardType::Tiger, "tiger"},
};

struct Card {
  static std::optional<Card> Parse(std::istringstream& stream,
                                   bool fatal = true);

  static const std::vector<Offset>& GetMoves(const CardType card) {
    return CardToMoves.at(card);
  }
  static Color GetColor(const CardType card) { return CardToColor.at(card); }
  static std::string GetName(const CardType card) {
    return CardToString.at(card);
  }

  Color GetColor() const { return GetColor(Type); }
  const std::vector<Offset>& GetMoves() const { return GetMoves(Type); }
  std::string GetName() const { return GetName(Type); }
  bool HasMove(const Offset offset) const;

  bool operator==(const Card other) const { return Type == other.Type; }

  CardType Type = CardType::CardTypeCount;

  std::ostream& StreamRow(std::ostream& stream, const int8_t row,
                          const bool rotate = false) const;
  std::ostream& Stream(std::ostream& stream, const bool rotate = false) const;
};

}  // namespace Game

template <>
struct std::hash<Game::Card> {
  size_t operator()(const Game::Card card) const noexcept {
    return (size_t)card.Type;
  }
};

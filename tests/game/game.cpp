#include "game.h"

#include <format>
#include <iostream>

#include "../../src/game/game.h"

namespace Tests {
namespace Game {

using namespace ::Game;

static bool AssertEqual(const std::span<const Card, CARD_COUNT> cards,
                        const std::span<const Card, CARD_COUNT> expected) {
  if (cards[0] != expected[0]) {
    std::cerr << std::format("Set-aside card not equal! Expected {}, got {}",
                             expected[0].GetName(), cards[0].GetName())
              << std::endl;
    return false;
  }

  for (size_t playerNum = 0; playerNum < 2; playerNum++) {
    const Color player = playerNum == 0 ? TopPlayer : ~TopPlayer;

    const size_t beginOffset = 1 + playerNum * HAND_SIZE;
    const size_t endOffset = beginOffset + HAND_SIZE;

    if (!std::is_permutation(cards.begin() + beginOffset,
                             cards.begin() + endOffset,
                             expected.begin() + beginOffset)) {
      std::cerr << "Hand {";
      for (Card card : cards) std::cerr << std::format("{}, ", card.GetName());
      std::cerr << "} is not equal to expected {";
      for (Card card : expected)
        std::cerr << std::format("{}, ", card.GetName());
      std::cerr << "}!" << std::endl;

      return false;
    }
  }

  return true;
}

static bool AssertEqual(const Board& board, const Board& expected) {
  if (board != expected) {
    std::cerr << "Boards are not equal!\nExpected" << expected << "Got:\n"
              << board << std::endl;
    return false;
  }

  return true;
}

static bool AssertEqual(const Color color, const Color expected) {
  if (color != expected) {
    std::cerr << "Expected initial player " << expected << " got " << color
              << "!" << std::endl;
    return false;
  }

  return true;
}

static bool AssertEqual(const Game::Game& game, const Game::Game& expected) {
  return AssertEqual(game.GetBoard(), expected.GetBoard()) &&
         AssertEqual(game.GetCards(), expected.GetCards()) &&
         AssertEqual(game.GetCurrentPlayer(), expected.GetCurrentPlayer());
}

int InitialStateConstructor() {
  constexpr size_t width = 3;
  constexpr size_t height = 4;

  constexpr std::array<Card, CARD_COUNT> cards = {
      Card{CardType::Crab}, Card{CardType::Mantis}, Card{CardType::Cobra},
      Card{CardType::Rooster}, Card{CardType::Cobra}};

  const Game::Game game = Game::Game(width, height, cards);

  return AssertEqual(game.GetBoard(), Board(width, height)) &&
                 AssertEqual(game.GetCards(), cards) &&
                 AssertEqual(game.GetCurrentPlayer(), cards[0].GetColor())
             ? 0
             : 1;
}

int BoardConstructor() {
  constexpr size_t width = 4;
  constexpr size_t height = 3;

  constexpr Tile empty = std::nullopt;
  constexpr Tile blueStudent = Piece{.Team = Color::Blue, .Master = false};
  constexpr Tile blueMaster = Piece{.Team = Color::Blue, .Master = true};
  constexpr Tile redStudent = Piece{.Team = Color::Red, .Master = true};
  constexpr Tile redMaster = Piece{.Team = Color::Red, .Master = true};

  const std::vector<Tile> grid = {
      empty, blueStudent, blueStudent, redStudent, redMaster, blueMaster,
      empty, redStudent,  empty,       empty,      empty,     empty};
  const Board board(std::move(grid), width, height);

  constexpr std::array<Card, CARD_COUNT> cards = {
      Card(CardType::Crab), Card(CardType::Goose), Card(CardType::Mantis),
      Card(CardType::Horse), Card(CardType::Elephant)};

  constexpr Color player = Color::Blue;

  const Game::Game game = Game::Game(
      Board(board), std::array<Card, CARD_COUNT>(cards), Color(player));

  return AssertEqual(game.GetBoard(), board) &&
                 AssertEqual(game.GetCards(), cards) &&
                 AssertEqual(game.GetCurrentPlayer(), player)
             ? 0
             : 1;
}

int CopyConstructor() {
  constexpr size_t width = 3;
  constexpr size_t height = 4;

  constexpr Tile empty = std::nullopt;
  constexpr Tile blueStudent = Piece{.Team = Color::Blue, .Master = false};
  constexpr Tile blueMaster = Piece{.Team = Color::Blue, .Master = true};
  constexpr Tile redStudent = Piece{.Team = Color::Red, .Master = true};
  constexpr Tile redMaster = Piece{.Team = Color::Red, .Master = true};

  const std::vector<Tile> grid = {empty, blueStudent, redStudent, redStudent,
                                  empty, blueMaster,  empty,      redStudent,
                                  empty, blueStudent, empty,      redMaster};
  Board board(std::move(grid), width, height);

  std::array<Card, CARD_COUNT> cards = {
      Card(CardType::Ox), Card(CardType::Crab), Card(CardType::Crab),
      Card(CardType::Eel), Card(CardType::Frog)};

  Color player = Color::Red;

  const Game::Game game =
      Game::Game(std::move(board), std::move(cards), std::move(player));

  return AssertEqual(game, Game::Game(game)) ? 0 : 1;
}

int MoveConstructor() {
  constexpr size_t width = 3;
  constexpr size_t height = 4;

  constexpr Tile empty = std::nullopt;
  constexpr Tile blueStudent = Piece{.Team = Color::Blue, .Master = false};
  constexpr Tile blueMaster = Piece{.Team = Color::Blue, .Master = true};
  constexpr Tile redStudent = Piece{.Team = Color::Red, .Master = true};
  constexpr Tile redMaster = Piece{.Team = Color::Red, .Master = true};

  const std::vector<Tile> grid = {empty, blueStudent, redStudent, redStudent,
                                  empty, blueMaster,  empty,      redStudent,
                                  empty, blueStudent, empty,      redMaster};
  Board board(std::move(grid), width, height);

  std::array<Card, CARD_COUNT> cards = {
      Card(CardType::Ox), Card(CardType::Crab), Card(CardType::Crab),
      Card(CardType::Eel), Card(CardType::Frog)};

  Color player = Color::Red;

  const Game::Game game = Game::Game(
      Board(board), std::array<Card, CARD_COUNT>(cards), Color(player));
  const Game::Game copiedGame =
      Game::Game(std::move(board), std::move(cards), std::move(player));

  return AssertEqual(game, Game::Game(std::move(copiedGame))) ? 0 : 1;
}

int WithRandomCards() {
  constexpr size_t width = 4;
  constexpr size_t height = 5;

  const Game::Game game = Game::Game::WithRandomCards(width, height, true);

  const std::span<const Card, CARD_COUNT> cards = game.GetCards();
  for (auto cardsIt = cards.begin(); cardsIt != cards.end() - 1; cardsIt++) {
    if (std::find(cardsIt + 1, cards.end(), *cardsIt) != cards.end()) {
      std::cerr << "A card appeared multiple times!" << std::endl;
      return false;
    }
  }

  return AssertEqual(game.GetBoard(), Board(width, height)) &&
                 AssertEqual(game.GetCurrentPlayer(),
                             game.GetSetAsideCard().GetColor())
             ? 0
             : 1;
}

}  // namespace Game
}  // namespace Tests

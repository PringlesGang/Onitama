#include "game.h"

#include <format>
#include <iostream>

#include "../../src/game/card.h"
#include "../assertEqual.h"

namespace Tests {
namespace Game {

using namespace ::Game;

static constexpr int Pass = 0;
static constexpr int Fail = 1;

static constexpr Tile EmptyTile = std::nullopt;
static constexpr Tile BlueStudent = Piece{.Team = Color::Blue, .Master = false};
static constexpr Tile BlueMaster = Piece{.Team = Color::Blue, .Master = true};
static constexpr Tile RedStudent = Piece{.Team = Color::Red, .Master = false};
static constexpr Tile RedMaster = Piece{.Team = Color::Red, .Master = true};

static constexpr size_t TemplateBoardWidth = 3;
static constexpr size_t TemplateBoardHeight = 5;
static const std::vector<Tile>&& TemplateGrid = {
    EmptyTile,   EmptyTile,   RedMaster,  RedStudent, EmptyTile,
    EmptyTile,   RedStudent,  BlueMaster, EmptyTile,  EmptyTile,
    BlueStudent, BlueStudent, EmptyTile,  EmptyTile,  EmptyTile};
static constexpr std::array<Game::Card, CARD_COUNT> TemplateCards = {
    Game::Card(CardType::Rooster), Game::Card(CardType::Goose),
    Game::Card(CardType::Dragon), Game::Card(CardType::Cobra),
    Game::Card(CardType::Elephant)};
static constexpr Color TemplatePlayer = Color::Red;

static std::unique_ptr<const Board> TemplateBoard;
static std::unique_ptr<const Game::Game> TemplateGame;

void Init() {
  TemplateBoard = std::make_unique<const Board>(
      TemplateGrid, TemplateBoardWidth, TemplateBoardHeight);

  TemplateGame = std::make_unique<const Game::Game>(Game::Game(
      Board(*TemplateBoard), std::array<Game::Card, CARD_COUNT>(TemplateCards),
      Color(TemplatePlayer)));
}

int InitialStateConstructor() {
  constexpr size_t width = 3;
  constexpr size_t height = 4;

  constexpr std::array<Game::Card, CARD_COUNT> cards = {
      Game::Card{CardType::Crab}, Game::Card{CardType::Mantis},
      Game::Card{CardType::Cobra}, Game::Card{CardType::Rooster},
      Game::Card{CardType::Cobra}};

  const Game::Game game = Game::Game(width, height, cards);

  return AssertEqual(game.GetBoard(), Board(width, height)) &&
                 AssertEqual(game.GetCards(), cards) &&
                 AssertEqual(game.GetCurrentPlayer(), cards[0].GetColor())
             ? Pass
             : Fail;
}

int BoardConstructor() {
  return AssertEqual(TemplateGame->GetBoard(), *TemplateBoard) &&
                 AssertEqual(TemplateGame->GetCards(), TemplateCards) &&
                 AssertEqual(TemplateGame->GetCurrentPlayer(), TemplatePlayer)
             ? Pass
             : Fail;
}

int CopyConstructor() {
  return AssertEqual(*TemplateGame, Game::Game(*TemplateGame)) ? Pass : Fail;
}

int MoveConstructor() {
  const Game::Game copiedGame = Game::Game(*TemplateGame);
  return AssertEqual(*TemplateGame, Game::Game(std::move(copiedGame))) ? Pass
                                                                       : Fail;
}

int WithRandomCards() {
  constexpr size_t width = 4;
  constexpr size_t height = 5;

  const Game::Game game = Game::Game::WithRandomCards(width, height, false);

  const std::span<const Game::Card, CARD_COUNT> cards = game.GetCards();
  for (auto cardsIt = cards.begin(); cardsIt != cards.end() - 1; cardsIt++) {
    if (std::find(cardsIt + 1, cards.end(), *cardsIt) != cards.end()) {
      std::cerr << "A card appeared multiple times!" << std::endl;
      return Fail;
    }
  }

  return AssertEqual(game.GetBoard(), Board(width, height)) &&
                 AssertEqual(game.GetCurrentPlayer(),
                             game.GetSetAsideCard().GetColor())
             ? Pass
             : Fail;
}

int FromSerialization() {
  // BZQ4wYVbCkd
  const GameSerialization serialization = GameSerialization(
      "1011001010000111000110000011000010101011011000010100100011101");
  Game::Game game = Game::Game::FromSerialization(std::move(serialization));

  constexpr size_t expectedWidth = 3;
  constexpr size_t expectedHeight = 5;
  const std::vector<Tile>&& expectedGrid = {
      EmptyTile,   EmptyTile,   RedMaster,  RedStudent, EmptyTile,
      EmptyTile,   RedStudent,  BlueMaster, EmptyTile,  EmptyTile,
      BlueStudent, BlueStudent, EmptyTile,  EmptyTile,  EmptyTile};

  std::array<Game::Card, CARD_COUNT>&& expectedCards = {
      Game::Card(CardType::Rooster), Game::Card(CardType::Goose),
      Game::Card(CardType::Dragon), Game::Card(CardType::Cobra),
      Game::Card(CardType::Elephant)};

  Color expectedPlayer = Color::Red;

  const Game::Game expected =
      Game::Game(Board(std::move(expectedGrid), expectedWidth, expectedHeight),
                 std::move(expectedCards), std::move(expectedPlayer));

  return AssertEqual(game, expected) ? Pass : Fail;
}

int ParseSerialization() {
  const std::string string = "BZQ4wYVbCkd";

  std::istringstream stream(string);
  std::optional<GameSerialization> result =
      Game::Game::ParseSerialization(stream);

  if (!result.has_value()) {
    std::cerr << "Failed to parse correct serialization!" << std::endl;
    return Fail;
  }

  const GameSerialization expected = GameSerialization(
      "1011001010000111000110000011000010101011011000010100100011101");
  if (result.value() != expected) {
    std::cerr << std::format("Parsed \"{}\" as \"{}\"; expected \"{}\"!",
                             string, result.value().to_string(),
                             expected.to_string())
              << std::endl;
    return Fail;
  }

  return Pass;
}

int ParseIncorrectSerialization() {
  const std::string string = "InvalidString!";

  std::istringstream stream(string);
  std::optional<GameSerialization> result =
      Game::Game::ParseSerialization(stream);

  if (result.has_value()) {
    std::cerr << std::format("Parsed invalid string \"{}\" as \"{}\"!", string,
                             result.value().to_string())
              << std::endl;
    return Fail;
  }

  return Pass;
}

int Equality() {
  if (TemplateGame == TemplateGame) return Pass;

  std::cerr << "Identity does not hold!" << std::endl;
  return Fail;
}

int Inequality() {
  if (TemplateGame != TemplateGame) {
    std::cerr << "Identity does not hold!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetHand() {
  const std::span<const Game::Card, CARD_COUNT> allCards =
      TemplateGame->GetCards();

  for (size_t playerNum = 0; playerNum < 2; playerNum++) {
    const Color player = playerNum == 0 ? TopPlayer : ~TopPlayer;
    const std::span<const Game::Card, HAND_SIZE> hand =
        TemplateGame->GetHand(player);

    const size_t offset = 1 + HAND_SIZE * playerNum;
    const std::span<const Game::Card, HAND_SIZE> expectedHand =
        std::span<const Game::Card, HAND_SIZE>(
            allCards.subspan(offset, HAND_SIZE));

    if (!AssertEqual(hand, expectedHand)) return Fail;
  }

  return Pass;
}

int GetCurrentHand() {
  for (size_t playerNum = 0; playerNum < 2; playerNum++) {
    Color player = playerNum == 0 ? TopPlayer : ~TopPlayer;

    const std::span<const Game::Card, HAND_SIZE> hand =
        TemplateGame->GetHand(player);

    const size_t offset = 1 + HAND_SIZE * playerNum;
    const std::span<const Game::Card, HAND_SIZE> expectedHand =
        std::span<const Game::Card, HAND_SIZE>(
            TemplateGame->GetCards().subspan(offset, HAND_SIZE));

    if (!AssertEqual(hand, expectedHand)) return Fail;
  }

  return Pass;
}

int GetSetAsideCard() {
  return AssertEqual(TemplateGame->GetSetAsideCard(),
                     TemplateGame->GetCards()[0])
             ? Pass
             : Fail;
}

int GetCurrentPlayer() {
  for (size_t playerNum = 0; playerNum < 2; playerNum++) {
    Color player = playerNum == 0 ? TopPlayer : ~TopPlayer;
    const Game::Game game = Game::Game(
        Board(*TemplateBoard),
        std::array<Game::Card, CARD_COUNT>(TemplateCards), std::move(player));

    if (!AssertEqual(game.GetCurrentPlayer(), player)) return Fail;
  }

  return Pass;
}

int GetDimensions() {
  const auto [width, height] = TemplateGame->GetDimensions();

  if (width != TemplateBoardWidth) {
    std::cerr << std::format("Expected width {}; got {}!", TemplateBoardWidth,
                             width)
              << std::endl;
    return Fail;
  }

  if (height != TemplateBoardHeight) {
    std::cerr << std::format("Expected height {}; got {}!", TemplateBoardHeight,
                             height)
              << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace Game
}  // namespace Tests

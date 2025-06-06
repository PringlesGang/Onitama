#include "game.h"

#include <format>
#include <iostream>

#include "../../src/game/card.h"
#include "../assertEqual.h"

namespace Tests {
namespace Game {
namespace Game {

using namespace ::Game;

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

  TemplateGame = std::make_unique<const Game::Game>(
      Board(*TemplateBoard), std::array<Game::Card, CARD_COUNT>(TemplateCards),
      Color(TemplatePlayer));
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
    const Color player = playerNum == 0 ? TopPlayer : BottomPlayer;
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
    Color player = playerNum == 0 ? TopPlayer : BottomPlayer;

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
    Color player = playerNum == 0 ? TopPlayer : BottomPlayer;
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

int GetPawnCoordinates() {
  if (TemplateGame->GetPawnCoordinates(Color::Red) !=
      TemplateBoard->GetPawnCoordinates(Color::Red)) {
    std::cerr << "Game and board disagree on red pawn coordinates!"
              << std::endl;
    return Fail;
  }

  if (TemplateGame->GetPawnCoordinates(Color::Blue) !=
      TemplateBoard->GetPawnCoordinates(Color::Blue)) {
    std::cerr << "Game and board disagree on blue pawn coordinates!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetCurrentPawnCoordinates() {
  if (TemplateGame->GetPawnCoordinates() !=
      TemplateGame->GetPawnCoordinates(TemplateGame->GetCurrentPlayer())) {
    std::cerr << "Current pawn coordinates are not equal to the pawn "
                 "coordinates of the current player!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int MasterCaptured() {
  if (TemplateGame->MasterCaptured(Color::Red) !=
      TemplateBoard->MasterCaptured(Color::Red)) {
    std::cerr << "Game and board disagree on red master captured!" << std::endl;
    return Fail;
  }

  if (TemplateGame->MasterCaptured(Color::Blue) !=
      TemplateBoard->MasterCaptured(Color::Blue)) {
    std::cerr << "Game and board disagree on blue master captured!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int CurrentMasterCaptured() {
  if (TemplateGame->MasterCaptured() !=
      TemplateGame->MasterCaptured(TemplateGame->GetCurrentPlayer())) {
    std::cerr << "Current master captured is not equal to the master captured "
                 "of the current player!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetPawnCount() {
  constexpr size_t expectedPawnCount = 3;
  const size_t redPawnCount = TemplateGame->GetPawnCount(Color::Red);

  if (redPawnCount != expectedPawnCount) {
    std::cerr << std::format("Expected red pawn count {}, got {}!",
                             expectedPawnCount, redPawnCount)
              << std::endl;
    return Fail;
  }

  const size_t bluePawnCount = TemplateGame->GetPawnCount(Color::Blue);
  if (bluePawnCount != expectedPawnCount) {
    std::cerr << std::format("Expected blue pawn count {}, got {}!",
                             expectedPawnCount, bluePawnCount)
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetCurrentPawnCount() {
  if (TemplateGame->GetPawnCount() !=
      TemplateGame->GetPawnCount(TemplateGame->GetCurrentPlayer())) {
    std::cerr
        << "Current pawn count is not equal to pawn count of current player!"
        << std::endl;
    return Fail;
  }

  return Pass;
}

int GetValidMoves() {
  const std::vector<Move> expectedValidMoves = {
      Move{0, Card(CardType::Goose), 2},  Move{1, Card(CardType::Goose), 1},
      Move{1, Card(CardType::Goose), 0},  Move{1, Card(CardType::Dragon), 0},
      Move{1, Card(CardType::Dragon), 1}, Move{2, Card(CardType::Goose), 1},
      Move{2, Card(CardType::Goose), 0},  Move{2, Card(CardType::Dragon), 0},
      Move{2, Card(CardType::Dragon), 1},
  };
  const std::vector<Move>& validMoves = TemplateGame->GetValidMoves();

  if (!std::is_permutation(expectedValidMoves.begin(), expectedValidMoves.end(),
                           validMoves.begin())) {
    std::cerr << "Valid moves not correct!\nExpected: ";
    for (const Move move : expectedValidMoves) {
      std::cerr << std::format("({},{},{}) ", move.PawnId,
                               move.UsedCard.GetName(), move.OffsetId);
    }

    std::cerr << "\nGot: ";
    for (const Move move : validMoves) {
      std::cerr << std::format("({},{},{}) ", move.PawnId,
                               move.UsedCard.GetName(), move.OffsetId);
    }
    std::cerr << std::endl;
    return Fail;
  }

  return Pass;
}

int HasValidMoves() {
  if (!TemplateGame->HasValidMoves()) {
    std::cerr << "Game with valid moves claims to not have any!" << std::endl;
    return Fail;
  }

  std::vector<Tile>&& grid = {BlueStudent, BlueMaster, RedMaster, RedStudent};
  Board&& board = Board(std::move(grid), 1, 4);

  std::array<Card, CARD_COUNT> cards;
  std::fill(cards.begin(), cards.end(), Card(CardType::Boar));

  Game::Game game(std::move(board), std::move(cards), Color::Red);

  if (game.HasValidMoves()) {
    std::cerr << "Game without valid moves claims to have some!" << std::endl;
    return Fail;
  }

  return Pass;
}

int IsValidMove() {
  constexpr Move validMove{0, Card(CardType::Goose), 2};
  if (!TemplateGame->IsValidMove(validMove)) {
    std::cerr << "Valid move was marked as invalid!" << std::endl;
    return Fail;
  }

  constexpr Move outOfBoard{0, Card(CardType::Dragon), 0};
  if (TemplateGame->IsValidMove(outOfBoard)) {
    std::cerr << "Move out of board marked as valid!" << std::endl;
    return Fail;
  }

  constexpr Move onFriendlyPawn{0, Card(CardType::Dragon), 3};
  if (TemplateGame->IsValidMove(onFriendlyPawn)) {
    std::cerr << "Move on top of pawn of same color marked as valid!"
              << std::endl;
    return Fail;
  }

  constexpr Move invalidCard{0, Card(CardType::Elephant), 3};
  if (TemplateGame->IsValidMove(invalidCard)) {
    std::cerr << "Move with non-owned card marked as valid!" << std::endl;
    return Fail;
  }

  constexpr Move invalidOffset{0, Card(CardType::Dragon), 4};
  if (TemplateGame->IsValidMove(invalidOffset)) {
    std::cerr << "Move with invalid offset marked as valid!" << std::endl;
    return Fail;
  }

  constexpr Move invalidPawn{3, Card(CardType::Goose), 0};
  if (TemplateGame->IsValidMove(invalidPawn)) {
    std::cerr << "Move with invalid pawn marked as valid!" << std::endl;
    return Fail;
  }

  return Pass;
}

int IsFinished() {
  if (TemplateGame->IsFinished() != TemplateBoard->IsFinished()) {
    std::cerr << "Game and board disagree in whether the game is finished!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int DoMove() {
  Game::Game game(*TemplateGame);
  constexpr Move move = Move{2, Card(CardType::Goose), 0};
  game.DoMove(move);

  std::vector<Tile> expectedGrid = TemplateGrid;
  expectedGrid[6] = EmptyTile;
  expectedGrid[10] = RedStudent;
  Board&& expectedBoard =
      Board(expectedGrid, TemplateBoardWidth, TemplateBoardHeight);

  std::array<Card, CARD_COUNT> expectedCards = TemplateCards;
  Card& usedCard = *std::find(expectedCards.begin(), expectedCards.end(),
                              Card(CardType::Goose));
  std::swap(expectedCards[0], usedCard);

  Game::Game expectedGame(std::move(expectedBoard), std::move(expectedCards),
                          ~TemplatePlayer);

  return AssertEqual(game, expectedGame) ? Pass : Fail;
}

int Serialize() {
  // BZQ4wYVbCkd
  constexpr GameSerialization expected = GameSerialization(
      "1011001010000111000110000011000010101011011000010100100011101");
  const GameSerialization serialization = TemplateGame->Serialize();

  if (expected != serialization) {
    std::cerr << std::format("Expected game serialization \"{}\", got \"{}\"!",
                             expected.to_string(), serialization.to_string())
              << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace Game
}  // namespace Game
}  // namespace Tests

#include "board.h"

#include <format>
#include <iostream>
#include <vector>

#include "../../src/game/board.h"
#include "../../src/util/coordinate.h"
#include "../assertEqual.h"

namespace Tests {
namespace Game {
namespace Board {

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

static std::unique_ptr<const ::Game::Board> TemplateBoard;

void Init() {
  TemplateBoard = std::make_unique<const ::Game::Board>(
      TemplateGrid, TemplateBoardWidth, TemplateBoardHeight);
}

int InitialStateConstructorEvenWidth() {
  constexpr size_t expectedWidth = 4;
  constexpr size_t expectedHeight = 3;

  const ::Game::Board board(expectedWidth, expectedHeight);

  const std::pair dimensions = board.GetDimensions();
  if (dimensions != std::pair(expectedWidth, expectedHeight)) {
    std::cerr << std::format("Expected dimensions {}x{}; got {}x{}!",
                             expectedWidth, expectedHeight, dimensions.first,
                             dimensions.second)
              << std::endl;
    return Fail;
  }

  const Coordinate redTemple = board.GetTemple(Color::Red);
  if (redTemple != Coordinate((expectedWidth - 1) / 2, 0)) {
    std::cerr << std::format("Expected red temple ({}, {}); got ({}, {})!",
                             (expectedWidth - 1) / 2, 0, redTemple.x,
                             redTemple.y)
              << std::endl;
    return Fail;
  }

  const Coordinate blueTemple = board.GetTemple(Color::Blue);
  if (blueTemple != Coordinate(expectedWidth / 2, expectedHeight - 1)) {
    std::cerr << std::format("Expected blue temple ({}, {}); got ({}, {})!",
                             expectedWidth / 2, expectedHeight - 1,
                             blueTemple.x, blueTemple.y)
              << std::endl;
    return Fail;
  }

  const std::vector<Coordinate>& redCoordinates =
      board.GetPawnCoordinates(Color::Red);
  if (redCoordinates.size() != expectedWidth) {
    std::cerr << std::format("Expected {} red pawns; got {}!", expectedWidth,
                             redCoordinates.size())
              << std::endl;
    return Fail;
  }

  if (redCoordinates[0] != redTemple) {
    std::cerr << std::format("Expected red master on ({}, {}); got ({}, {})!",
                             redTemple.x, redTemple.y, redCoordinates[0].x,
                             redCoordinates[0].y)
              << std::endl;
    return Fail;
  }

  for (size_t x = 0; x < expectedWidth; x++) {
    if (std::find(redCoordinates.begin(), redCoordinates.end(),
                  Coordinate(x, 0)) == redCoordinates.end()) {
      std::cerr << std::format(
                       "Expected but did not find red student on ({}, 0)!", x)
                << std::endl;
      return Fail;
    }
  }

  const std::vector<Coordinate>& blueCoordinates =
      board.GetPawnCoordinates(Color::Blue);
  if (blueCoordinates.size() != expectedWidth) {
    std::cerr << std::format("Expected {} blue pawns; got {}!", expectedWidth,
                             blueCoordinates.size())
              << std::endl;
    return Fail;
  }

  if (blueCoordinates[0] != blueTemple) {
    std::cerr << std::format("Expected blue master on ({}, {}); got ({}, {})!",
                             blueTemple.x, blueTemple.y, blueCoordinates[0].x,
                             blueCoordinates[0].y)
              << std::endl;
    return Fail;
  }

  for (size_t x = 0; x < expectedWidth; x++) {
    if (std::find(blueCoordinates.begin(), blueCoordinates.end(),
                  Coordinate(x, expectedHeight - 1)) == blueCoordinates.end()) {
      std::cerr << std::format(
                       "Expected but did not find blue student on ({}, {})!", x,
                       expectedHeight - 1)
                << std::endl;
      return Fail;
    }
  }

  return Pass;
}

int InitialStateConstructorOddWidth() {
  constexpr size_t expectedWidth = 5;
  constexpr size_t expectedHeight = 4;

  const ::Game::Board board(expectedWidth, expectedHeight);

  const std::pair dimensions = board.GetDimensions();
  if (dimensions != std::pair(expectedWidth, expectedHeight)) {
    std::cerr << std::format("Expected dimensions {}x{}; got {}x{}!",
                             expectedWidth, expectedHeight, dimensions.first,
                             dimensions.second)
              << std::endl;
    return Fail;
  }

  const Coordinate redTemple = board.GetTemple(Color::Red);
  if (redTemple != Coordinate((expectedWidth - 1) / 2, 0)) {
    std::cerr << std::format("Expected red temple ({}, {}); got ({}, {})!",
                             (expectedWidth - 1) / 2, 0, redTemple.x,
                             redTemple.y)
              << std::endl;
    return Fail;
  }

  const Coordinate blueTemple = board.GetTemple(Color::Blue);
  if (blueTemple != Coordinate(expectedWidth / 2, expectedHeight - 1)) {
    std::cerr << std::format("Expected blue temple ({}, {}); got ({}, {})!",
                             expectedWidth / 2, expectedHeight - 1,
                             blueTemple.x, blueTemple.y)
              << std::endl;
    return Fail;
  }

  const std::vector<Coordinate>& redCoordinates =
      board.GetPawnCoordinates(Color::Red);
  if (redCoordinates.size() != expectedWidth) {
    std::cerr << std::format("Expected {} red pawns; got {}!", expectedWidth,
                             redCoordinates.size())
              << std::endl;
    return Fail;
  }

  if (redCoordinates[0] != redTemple) {
    std::cerr << std::format("Expected red master on ({}, {}); got ({}, {})!",
                             redTemple.x, redTemple.y, redCoordinates[0].x,
                             redCoordinates[0].y)
              << std::endl;
    return Fail;
  }

  for (size_t x = 0; x < expectedWidth; x++) {
    if (std::find(redCoordinates.begin(), redCoordinates.end(),
                  Coordinate(x, 0)) == redCoordinates.end()) {
      std::cerr << std::format(
                       "Expected but did not find red student on ({}, 0)!", x)
                << std::endl;
      return Fail;
    }
  }

  const std::vector<Coordinate>& blueCoordinates =
      board.GetPawnCoordinates(Color::Blue);
  if (blueCoordinates.size() != expectedWidth) {
    std::cerr << std::format("Expected {} blue pawns; got {}!", expectedWidth,
                             blueCoordinates.size())
              << std::endl;
    return Fail;
  }

  if (blueCoordinates[0] != blueTemple) {
    std::cerr << std::format("Expected blue master on ({}, {}); got ({}, {})!",
                             blueTemple.x, blueTemple.y, blueCoordinates[0].x,
                             blueCoordinates[0].y)
              << std::endl;
    return Fail;
  }

  for (size_t x = 0; x < expectedWidth; x++) {
    if (std::find(blueCoordinates.begin(), blueCoordinates.end(),
                  Coordinate(x, expectedHeight - 1)) == blueCoordinates.end()) {
      std::cerr << std::format(
                       "Expected but did not find blue student on ({}, {})!", x,
                       expectedHeight - 1)
                << std::endl;
      return Fail;
    }
  }

  return Pass;
}

static bool CompareBoard(const ::Game::Board& expected,
                         const ::Game::Board& got) {
  const std::pair expectedDimensions = expected.GetDimensions();
  const std::pair gotDimensions = got.GetDimensions();
  if (gotDimensions != expectedDimensions) {
    std::cerr << std::format("Expected dimensions {}x{}; got {}x{}!",
                             expectedDimensions.first,
                             expectedDimensions.second, gotDimensions.first,
                             gotDimensions.second)
              << std::endl;
    return false;
  }

  const std::span<const Tile> expectedGrid = expected.GetGrid();
  const std::span<const Tile> gotGrid = got.GetGrid();
  if (expectedGrid.size() != gotGrid.size()) {
    std::cerr << std::format("Expected {} tiles; got {}!", expectedGrid.size(),
                             gotGrid.size())
              << std::endl;
    return false;
  }

  for (size_t i = 0; i < gotGrid.size(); i++) {
    if (gotGrid[i] != expectedGrid[i]) {
      const size_t x = i % TemplateBoardWidth;
      const size_t y = i / TemplateBoardWidth;

      std::cerr << std::format("Expected tile ({}, {}) to be {}; got {}!", x, y,
                               to_string(expectedGrid[i]),
                               to_string(gotGrid[i]))
                << std::endl;
      return false;
    }
  }

  return true;
}

int GridConstructor() {
  const ::Game::Board board(TemplateGrid, TemplateBoardWidth,
                            TemplateBoardHeight);

  return CompareBoard(*TemplateBoard, board) ? Pass : Fail;
}

int CopyConstructor() {
  const ::Game::Board board(*TemplateBoard);

  return CompareBoard(*TemplateBoard, board) ? Pass : Fail;
}

int MoveConstructor() {
  ::Game::Board movedBoard(*TemplateBoard);
  const ::Game::Board board(std::move(movedBoard));

  return CompareBoard(*TemplateBoard, board) ? Pass : Fail;
}

int EqualityOperator() {
  if (!(*TemplateBoard == *TemplateBoard)) {
    std::cerr << "Two equal boards were deemed inequal!" << std::endl;
    return Fail;
  }

  ::Game::Board board(*TemplateBoard);
  board.DoMove({2, 0}, {-1, 0});

  if (*TemplateBoard == board) {
    std::cerr << "Two inequal board were deemed equal!" << std::endl;
    return Fail;
  }

  return Pass;
}

int Reset() {
  ::Game::Board board(*TemplateBoard);
  board.Reset();

  ::Game::Board initialState(TemplateBoardWidth, TemplateBoardHeight);

  return CompareBoard(initialState, board) ? Pass : Fail;
}

int DoMove() {
  const Coordinate origin = {2, 3};
  const Offset offset = {-2, -2};
  const Coordinate destination = origin.try_add(offset).value();

  ::Game::Board board(*TemplateBoard);
  board.DoMove(origin, offset);

  const std::pair dimensions = board.GetDimensions();
  if (dimensions != std::pair(TemplateBoardWidth, TemplateBoardHeight)) {
    std::cerr << std::format("Expected dimensions {}x{}; got {}x{}!",
                             TemplateBoardWidth, TemplateBoardHeight,
                             dimensions.first, dimensions.second)
              << std::endl;
    return Fail;
  }

  const std::span<const Tile> grid = board.GetGrid();
  if (grid.size() != TemplateGrid.size()) {
    std::cerr << std::format("Expected {} tiles; got {}!", TemplateGrid.size(),
                             grid.size())
              << std::endl;
    return Fail;
  }

  for (size_t i = 0; i < grid.size(); i++) {
    const Coordinate coordinate = {i % TemplateBoardWidth,
                                   i / TemplateBoardWidth};
    if (coordinate == origin || coordinate == destination) continue;

    if (grid[i] != TemplateGrid[i]) {
      std::cerr
          << std::format(
                 "Expected tile ({}, {}) to be unchanged from {}; got {}!",
                 coordinate.x, coordinate.y, to_string(TemplateGrid[i]),
                 to_string(grid[i]))
          << std::endl;
      return Fail;
    }
  }

  const size_t originId = origin.x + origin.y * TemplateBoardWidth;
  if (grid[originId] != std::nullopt) {
    std::cerr << "Origin tile was not cleared out!" << std::endl;
    return Fail;
  }

  const size_t destId = destination.x + destination.y * TemplateBoardWidth;
  if (grid[destId] != TemplateGrid[originId]) {
    std::cerr << "Origin was not properly moved to destination!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetGrid() {
  std::span<const Tile> const grid = TemplateBoard->GetGrid();
  if (!std::equal(grid.begin(), grid.end(), TemplateGrid.begin(),
                  TemplateGrid.end())) {
    std::cerr << "Did not properly return its own grid!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetTile() {
  if (TemplateBoard->GetTile({1, 0}).value() != std::nullopt) {
    std::cerr << "Empty tile was not marked empty!" << std::endl;
    return Fail;
  }

  if (TemplateBoard->GetTile({2, 0}).value() !=
      ::Game::Piece{.Team = Color::Red, .Master = true}) {
    std::cerr << "Red master was not marked as such!" << std::endl;
    return Fail;
  }

  if (TemplateBoard->GetTile({0, 2}).value() !=
      ::Game::Piece{.Team = Color::Red, .Master = false}) {
    std::cerr << "Red student was not marked as such!" << std::endl;
    return Fail;
  }

  if (TemplateBoard->GetTile({1, 2}).value() !=
      ::Game::Piece{.Team = Color::Blue, .Master = true}) {
    std::cerr << "Blue master was not marked as such!" << std::endl;
    return Fail;
  }

  if (TemplateBoard->GetTile({2, 3}).value() !=
      ::Game::Piece{.Team = Color::Blue, .Master = false}) {
    std::cerr << "Blue student was not marked as such!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetRow() {
  constexpr size_t rowNum = 2;

  const std::span<const Tile> row = TemplateBoard->GetRow(rowNum).value();
  if (row.size() != TemplateBoardWidth) {
    std::cerr << std::format("Expected row width {}; got {}!",
                             TemplateBoardWidth, row.size())
              << std::endl;
    return Fail;
  }

  const std::span<const Tile> grid = TemplateBoard->GetGrid();

  if (!std::equal(row.begin(), row.end(),
                  grid.begin() + (rowNum * TemplateBoardWidth))) {
    std::cerr << "Rows were not equal!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetPawnCoordinates() {
  const std::vector<Coordinate> expectedRedCoordinates = {
      {2, 0}, {0, 1}, {0, 2}};
  const std::vector<Coordinate>& redCoordinates =
      TemplateBoard->GetPawnCoordinates(Color::Red);
  if (redCoordinates != expectedRedCoordinates) {
    std::cerr << "Unexpected red coordinates!\n Expected: ";
    for (const Coordinate coord : expectedRedCoordinates)
      std::cerr << std::format("({},{}) ", coord.x, coord.y);

    std::cerr << "\n Got: ";
    for (const Coordinate coord : redCoordinates)
      std::cerr << std::format("({},{}) ", coord.x, coord.y);

    std::cerr << std::endl;
    return Fail;
  }

  const std::vector<Coordinate> expectedBlueCoordinates = {
      {1, 2}, {1, 3}, {2, 3}};
  const std::vector<Coordinate>& blueCoordinates =
      TemplateBoard->GetPawnCoordinates(Color::Blue);
  if (blueCoordinates != expectedBlueCoordinates) {
    std::cerr << "Unexpected blue coordinates!\n Expected: ";
    for (const Coordinate coord : expectedBlueCoordinates)
      std::cerr << std::format("({},{}) ", coord.x, coord.y);

    std::cerr << "\n Got: ";
    for (const Coordinate coord : blueCoordinates)
      std::cerr << std::format("({},{}) ", coord.x, coord.y);

    std::cerr << std::endl;
    return Fail;
  }

  return Pass;
}

int GetDimensions() {
  const std::pair dimensions = TemplateBoard->GetDimensions();
  if (dimensions != std::pair(TemplateBoardWidth, TemplateBoardHeight)) {
    std::cerr << std::format("Expected dimensions {}x{}; got {}x{}!",
                             TemplateBoardWidth, TemplateBoardHeight,
                             dimensions.first, dimensions.second)
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetTemple() {
  const Coordinate expectedRedTempleOdd((TemplateBoardWidth - 1) / 2, 0);
  const Coordinate redTempleOdd = TemplateBoard->GetTemple(Color::Red);
  if (redTempleOdd != expectedRedTempleOdd) {
    std::cerr << std::format(
                     "Expected odd red temple to be on ({}, {}); got ({}, {})!",
                     expectedRedTempleOdd.x, expectedRedTempleOdd.y,
                     redTempleOdd.x, redTempleOdd.y)
              << std::endl;
    return Fail;
  }

  const Coordinate expectedBlueTempleOdd(TemplateBoardWidth / 2,
                                         TemplateBoardHeight - 1);
  const Coordinate blueTempleOdd = TemplateBoard->GetTemple(Color::Blue);
  if (blueTempleOdd != expectedBlueTempleOdd) {
    std::cerr
        << std::format(
               "Expected odd blue temple to be on ({}, {}); got ({}, {})!",
               expectedBlueTempleOdd.x, expectedBlueTempleOdd.y,
               blueTempleOdd.x, blueTempleOdd.y)
        << std::endl;
    return Fail;
  }

  constexpr size_t evenWidth = 4;
  constexpr size_t evenHeight = 5;
  const ::Game::Board evenBoard(evenWidth, evenHeight);

  const Coordinate expectedRedTempleEven((evenWidth - 1) / 2, 0);
  const Coordinate redTempleEven = evenBoard.GetTemple(Color::Red);
  if (redTempleEven != expectedRedTempleEven) {
    std::cerr
        << std::format(
               "Expected even red temple to be on ({}, {}); got ({}, {})!",
               expectedRedTempleEven.x, expectedRedTempleEven.y,
               redTempleEven.x, redTempleEven.y)
        << std::endl;
    return Fail;
  }

  const Coordinate expectedBlueTempleEven(evenWidth / 2, evenHeight - 1);
  const Coordinate blueTempleEven = evenBoard.GetTemple(Color::Blue);
  if (blueTempleEven != expectedBlueTempleEven) {
    std::cerr
        << std::format(
               "Expected even blue temple to be on ({}, {}); got ({}, {})!",
               expectedBlueTempleEven.x, expectedBlueTempleEven.y,
               blueTempleEven.x, blueTempleEven.y)
        << std::endl;
    return Fail;
  }

  return Pass;
}

int MasterCaptured() {
  if (TemplateBoard->MasterCaptured(Color::Red)) {
    std::cerr << "Red master was marked captured on board with both."
              << std::endl;
    return Fail;
  }

  if (TemplateBoard->MasterCaptured(Color::Blue)) {
    std::cerr << "Blue master was marked captured on board with both."
              << std::endl;
    return Fail;
  }

  constexpr size_t redMasterId = 2;
  constexpr size_t blueMasterId = 7;

  std::vector<Tile> copyGrid = TemplateGrid;

  copyGrid[redMasterId].reset();
  const ::Game::Board redlessBoard(copyGrid, TemplateBoardWidth,
                                   TemplateBoardHeight);

  if (!redlessBoard.MasterCaptured(Color::Red)) {
    std::cerr << "Red master was marked not-captured on board with only the "
                 "blue master."
              << std::endl;
    return Fail;
  }

  if (redlessBoard.MasterCaptured(Color::Blue)) {
    std::cerr
        << "Blue master was marked captured on board with only the blue master."
        << std::endl;
    return Fail;
  }

  copyGrid[blueMasterId].reset();
  const ::Game::Board neitherBoard(copyGrid, TemplateBoardWidth,
                                   TemplateBoardHeight);

  if (!neitherBoard.MasterCaptured(Color::Red)) {
    std::cerr
        << "Red master was marked not-captured on board with neither master."
        << std::endl;
    return Fail;
  }

  if (!neitherBoard.MasterCaptured(Color::Blue)) {
    std::cerr
        << "Blue master was marked not-captured on board with neither master."
        << std::endl;
    return Fail;
  }

  copyGrid[redMasterId] = Piece{.Team = Color::Red, .Master = true};
  const ::Game::Board bluelessBoard(copyGrid, TemplateBoardWidth,
                                    TemplateBoardHeight);

  if (bluelessBoard.MasterCaptured(Color::Red)) {
    std::cerr
        << "Red master was marked captured on board with only the red master."
        << std::endl;
    return Fail;
  }

  if (!bluelessBoard.MasterCaptured(Color::Blue)) {
    std::cerr << "Blue master was marked not-captured on board with only the "
                 "red master."
              << std::endl;
    return Fail;
  }

  return Pass;
}

int OnBoard() {
  for (size_t x = 0; x < TemplateBoardWidth; x++) {
    for (size_t y = 0; y < TemplateBoardHeight; y++) {
      if (!TemplateBoard->OnBoard(Coordinate(x, y))) {
        std::cerr
            << std::format(
                   "({}, {}) was mistakenly marked as not on a {}x{} board!", x,
                   y, TemplateBoardWidth, TemplateBoardHeight)
            << std::endl;
        return Fail;
      }
    }
  }

  for (size_t y = 0; y < TemplateBoardHeight; y++) {
    if (TemplateBoard->OnBoard(Coordinate(TemplateBoardWidth, y))) {
      std::cerr << std::format(
                       "({}, {}) was mistakenly marked as on a {}x{} board!",
                       TemplateBoardWidth, y, TemplateBoardWidth,
                       TemplateBoardHeight)
                << std::endl;
      return Fail;
    }
  }

  for (size_t x = 0; x < TemplateBoardWidth; x++) {
    if (TemplateBoard->OnBoard(Coordinate(x, TemplateBoardHeight))) {
      std::cerr << std::format(
                       "({}, {}) was mistakenly marked as on a {}x{} board!", x,
                       TemplateBoardHeight, TemplateBoardWidth,
                       TemplateBoardHeight)
                << std::endl;
      return Fail;
    }
  }

  if (TemplateBoard->OnBoard(
          Coordinate(TemplateBoardWidth, TemplateBoardHeight))) {
    std::cerr << std::format(
                     "({}, {}) was mistakenly marked as on a {}x{} board!",
                     TemplateBoardWidth, TemplateBoardHeight,
                     TemplateBoardWidth, TemplateBoardHeight)
              << std::endl;
    return Fail;
  }

  return Pass;
}

int IsFinished() {
  if (TemplateBoard->IsFinished()) {
    std::cerr << "Unfinished board marked as finished!" << std::endl;
    return Fail;
  }

  const Coordinate redMasterLocation =
      TemplateBoard->GetPawnCoordinates(Color::Red)[0];
  const Coordinate blueMasterLocation =
      TemplateBoard->GetPawnCoordinates(Color::Blue)[0];
  const size_t redMasterId =
      redMasterLocation.x + redMasterLocation.y * TemplateBoardWidth;
  const size_t blueMasterId =
      blueMasterLocation.x + blueMasterLocation.y * TemplateBoardWidth;

  const Coordinate redStudentLocation =
      TemplateBoard->GetPawnCoordinates(Color::Red)[1];
  const Coordinate blueStudentLocation =
      TemplateBoard->GetPawnCoordinates(Color::Blue)[1];
  const size_t redStudentId =
      redStudentLocation.x + redStudentLocation.y * TemplateBoardWidth;
  const size_t blueStudentId =
      blueStudentLocation.x + blueStudentLocation.y * TemplateBoardWidth;

  const Coordinate redTemple = TemplateBoard->GetTemple(Color::Red);
  const Coordinate blueTemple = TemplateBoard->GetTemple(Color::Blue);
  const size_t redTempleId = redTemple.x + redTemple.y * TemplateBoardWidth;
  const size_t blueTempleId = blueTemple.x + blueTemple.y * TemplateBoardWidth;

  std::vector<Tile> grid = TemplateGrid;
  grid[blueTempleId] = Piece{.Team = Color::Red, .Master = true};
  grid[redMasterId].reset();
  const ::Game::Board redReachedTemple(grid, TemplateBoardWidth,
                                       TemplateBoardHeight);
  if (!redReachedTemple.IsFinished()) {
    std::cerr
        << "Game where red reached the blue temple was not deemed finished!"
        << std::endl;
    return Fail;
  }

  grid = TemplateGrid;
  grid[redTempleId] = Piece{.Team = Color::Blue, .Master = true};
  grid[blueMasterId].reset();
  const ::Game::Board blueReachedTemple(grid, TemplateBoardWidth,
                                        TemplateBoardHeight);
  if (!blueReachedTemple.IsFinished()) {
    std::cerr
        << "Game where blue reached the red temple was not deemed finished!"
        << std::endl;
    return Fail;
  }

  grid = TemplateGrid;
  grid[blueMasterId].reset();
  const ::Game::Board blueCaptured(grid, TemplateBoardWidth,
                                   TemplateBoardHeight);
  if (!blueCaptured.IsFinished()) {
    std::cerr
        << "Game where the blue master is captured was not deemed finished!"
        << std::endl;
    return Fail;
  }

  grid = TemplateGrid;
  grid[redMasterId].reset();
  const ::Game::Board redCaptured(grid, TemplateBoardWidth,
                                  TemplateBoardHeight);
  if (!blueCaptured.IsFinished()) {
    std::cerr
        << "Game where the red master is captured was not deemed finished!"
        << std::endl;
    return Fail;
  }

  grid = TemplateGrid;
  grid[redStudentId].reset();
  grid[blueTempleId] = Piece{.Team = Color::Red, .Master = false};
  const ::Game::Board redStudentTemple(grid, TemplateBoardWidth,
                                       TemplateBoardHeight);
  if (redStudentTemple.IsFinished()) {
    std::cerr << "Game where a red student reached the blue temple was deemed "
                 "finished!"
              << std::endl;
    return Fail;
  }

  grid = TemplateGrid;
  grid[blueStudentId].reset();
  grid[redTempleId] = Piece{.Team = Color::Blue, .Master = false};
  const ::Game::Board blueStudentTemple(grid, TemplateBoardWidth,
                                        TemplateBoardHeight);
  if (blueStudentTemple.IsFinished()) {
    std::cerr << "Game where a blue student reached the red temple was deemed "
                 "finished!"
              << std::endl;
    return Fail;
  }

  if (::Game::Board(TemplateBoardWidth, TemplateBoardHeight).IsFinished()) {
    std::cerr << "Initial state was deemed finished!" << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace Board
}  // namespace Game
}  // namespace Tests
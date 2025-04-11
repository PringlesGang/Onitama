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

}  // namespace Board
}  // namespace Game
}  // namespace Tests
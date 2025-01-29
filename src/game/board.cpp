#include "board.h"

namespace Game {

std::ostream& operator<<(std::ostream& stream, const Tile& tile) {
  const char character =
      !tile.has_value() ? '.'
                        : (tile->IsMaster() ? 'm' : 's') +
                              ('A' - 'a') * (tile->GetColor() == Color::Red);

  return stream << character;
}

Board::Board() { Reset(); }

Board::Board(const std::array<Tile, BOARD_SIZE>& grid) : Grid(grid) {}

Board::Board(const Board& other) : Grid(other.Grid) {}

void Board::Reset() {
  constexpr size_t center = BOARD_DIMENSIONS / 2;

  for (size_t x = 0; x < BOARD_DIMENSIONS; x++) {
    (*this)[0][x].emplace(TopPlayer, x == center);
    (*this)[BOARD_DIMENSIONS - 1][x].emplace(~TopPlayer, x == center);
    for (size_t y = 1; y < BOARD_DIMENSIONS - 1; y++) (*this)[y][x].reset();
  }
}

bool Board::DoMove(Coordinate source, Offset offset) {
  std::optional<Coordinate> destination = source.try_add(offset);
  if (!destination) return false;

  Tile& srcTile = (*this)[source];
  if (!srcTile) return false;

  Tile& destTile = (*this)[*destination];
  if (destTile && destTile->GetColor() == srcTile->GetColor()) return false;

  srcTile.swap(destTile);
  return true;
}

std::optional<Tile> Board::GetTile(Coordinate coordinate) const {
  if (!OnBoard(coordinate)) return std::optional<Tile>();

  return Grid[coordinate.y * BOARD_DIMENSIONS + coordinate.x];
}

std::optional<std::span<const Tile, BOARD_DIMENSIONS>> Board::GetRow(
    size_t row) const {
  if (row >= BOARD_DIMENSIONS)
    return std::optional<std::span<Tile, BOARD_DIMENSIONS>>();

  return std::span<const Tile, BOARD_DIMENSIONS>(&Grid[row * BOARD_DIMENSIONS],
                                                 BOARD_DIMENSIONS);
}

std::vector<Coordinate> Board::GetPieceCoordinates(Color color) const {
  std::vector<Coordinate> coordinates;

  for (size_t i = 0; i < BOARD_SIZE; i++) {
    if (Grid[i] && Grid[i]->GetColor() == color)
      coordinates.emplace_back(i % BOARD_DIMENSIONS, i / BOARD_DIMENSIONS);
  }

  return coordinates;
}

bool Board::OnBoard(Coordinate coordinate) const {
  return coordinate.x < BOARD_DIMENSIONS && coordinate.y < BOARD_DIMENSIONS;
}

std::optional<Color> Board::IsFinished() const {
  std::optional<Coordinate> redMasterPosition = std::optional<Coordinate>();
  std::optional<Coordinate> blueMasterPosition = std::optional<Coordinate>();

  for (size_t i = 0; i < BOARD_SIZE; i++) {
    Tile tile = Grid[i];
    if (tile && tile->IsMaster()) {
      Coordinate coordinate =
          Coordinate(i % BOARD_DIMENSIONS, i / BOARD_DIMENSIONS);

      if (tile->GetColor() == Color::Red)
        redMasterPosition = coordinate;
      else
        blueMasterPosition = coordinate;
    }
  }

  if (!redMasterPosition || !blueMasterPosition)
    return redMasterPosition ? Color::Red : Color::Blue;

  constexpr size_t center = BOARD_SIZE / 2;
  if (redMasterPosition.value() == Coordinate(center, BOARD_DIMENSIONS - 1))
    return Color::Red;
  if (blueMasterPosition.value() == Coordinate(center, 0)) return Color::Blue;

  return std::optional<Color>();
}

Tile& Board::operator[](Coordinate coordinate) {
  return Grid[coordinate.y * BOARD_DIMENSIONS + coordinate.x];
}

std::span<Tile, BOARD_DIMENSIONS> Board::operator[](size_t row) {
  return std::span<Tile, BOARD_DIMENSIONS>(&Grid[row * BOARD_DIMENSIONS],
                                           BOARD_DIMENSIONS);
}

std::ostream& operator<<(std::ostream& stream,
                         const std::span<const Tile, BOARD_DIMENSIONS> row) {
  for (const Tile& tile : row) stream << tile;
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Board& board) {
  stream << std::endl;
  for (size_t row = 0; row < BOARD_DIMENSIONS; row++) {
    stream << *board.GetRow(row) << std::endl;
  }

  return stream;
}

}  // namespace Game

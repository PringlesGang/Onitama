#include "board.h"

Coordinate::Coordinate(size_t x, size_t y) {
  this->x = x;
  this->y = y;
}

Coordinate Coordinate::operator+(Coordinate other) const {
  return Coordinate(this->x + other.x, this->y + other.y);
}

void Coordinate::operator+=(Coordinate other) {
  this->x += other.x;
  this->y += other.y;
}

bool Coordinate::operator==(Coordinate other) const {
  return this->x == other.x && this->y == other.y;
}

void Coordinate::operator=(Coordinate other) {
  this->x = other.x;
  this->y = other.y;
}

std::optional<Coordinate> Coordinate::try_add(Offset direction) {
  if (this->x < direction.dx || this->y < direction.dy)
    return std::optional<Coordinate>();

  return std::optional<Coordinate>(
      Coordinate(this->x + direction.dx, this->y + direction.dy));
}

Board::Board() { Reset(); }

Board::Board(const std::array<Tile, BOARD_SIZE>& grid) : Grid(grid) {}

Board::Board(const Board& other) : Grid(other.Grid) {}

void Board::Reset() {
  constexpr size_t center = BOARD_DIMENSIONS / 2 + 1;

  for (size_t x = 0; x < BOARD_DIMENSIONS; x++) {
    (*this)[0][x].emplace(Color::Red, x == center);
    (*this)[BOARD_DIMENSIONS - 1][x].emplace(Color::Blue, x == center);
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

  return std::optional<Tile>(
      Grid[coordinate.x * BOARD_DIMENSIONS + coordinate.x].value());
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

  constexpr size_t center = BOARD_SIZE / 2 + 1;
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

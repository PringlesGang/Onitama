#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <span>
#include <vector>

#include "../util/coordinate.h"
#include "piece.h"

constexpr size_t BOARD_DIMENSIONS = 5;
constexpr size_t BOARD_SIZE = BOARD_DIMENSIONS * BOARD_DIMENSIONS;

typedef std::optional<Piece> Tile;

std::ostream& operator<<(std::ostream& stream, const Tile& tile);

class Board {
 public:
  Board();
  Board(const std::array<Tile, BOARD_SIZE>& grid);
  Board(const Board& other);

  void Reset();
  bool DoMove(Coordinate source, Offset offset);

  std::optional<Tile> GetTile(Coordinate coordinate) const;
  std::vector<Coordinate> GetPieceCoordinates(Color color) const;

  bool OnBoard(Coordinate coordinate) const;
  std::optional<Color> IsFinished() const;

  friend std::ostream& operator<<(std::ostream& stream, const Board& board);

 private:
  std::array<Tile, BOARD_SIZE> Grid;

  Tile& operator[](Coordinate coordinate);
  std::span<Tile, BOARD_DIMENSIONS> operator[](size_t row);
};

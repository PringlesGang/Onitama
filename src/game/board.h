#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <span>
#include <vector>

#include "../constants.h"
#include "../util/coordinate.h"
#include "piece.h"

namespace Game {

typedef std::optional<Piece> Tile;

std::ostream& operator<<(std::ostream& stream, const Tile& tile);

class Board {
 public:
  Board();
  Board(const std::array<Tile, BOARD_SIZE>& grid);
  Board(const Board& other);

  void Reset();
  bool DoMove(const Coordinate source, const Offset offset);

  std::span<const Tile, BOARD_SIZE> GetGrid() const { return Grid; }
  std::optional<Tile> GetTile(const Coordinate coordinate) const;
  std::optional<std::span<const Tile, BOARD_DIMENSIONS>> GetRow(
      const size_t row) const;
  std::vector<Coordinate> GetPieceCoordinates(const Color color) const;

  bool OnBoard(const std::optional<const Coordinate> coordinate) const;
  std::optional<Color> IsFinished() const;

  std::ostream& StreamPlayer(std::ostream& stream, const Color player) const;
  std::ostream& StreamPlayerRow(std::ostream& stream, const Color player,
                                const size_t row, size_t& pawnIndex) const;

 private:
  std::array<Tile, BOARD_SIZE> Grid;

  Tile& operator[](const Coordinate coordinate);
  std::span<Tile, BOARD_DIMENSIONS> operator[](const size_t row);
};

std::ostream& operator<<(std::ostream& stream,
                         const std::span<const Tile, BOARD_DIMENSIONS> row);
std::ostream& operator<<(std::ostream& stream, const Board& board);

}  // namespace Game

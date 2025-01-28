#pragma once

#include <array>
#include <optional>
#include <span>

#include "card.h"
#include "piece.h"

constexpr size_t BOARD_DIMENSIONS = 5;
constexpr size_t BOARD_SIZE = BOARD_DIMENSIONS * BOARD_DIMENSIONS;

typedef std::optional<Piece> Tile;

struct Coordinate {
  Coordinate(size_t x, size_t y);

  Coordinate operator+(Coordinate other) const;
  void operator+=(Coordinate other);
  bool operator==(Coordinate other) const;
  void operator=(Coordinate other);

  std::optional<Coordinate> try_add(Offset direction);

  size_t x;
  size_t y;
};

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

 private:
  std::array<Tile, BOARD_SIZE> Grid;

  Tile& operator[](Coordinate coordinate);
  std::span<Tile, BOARD_DIMENSIONS> operator[](size_t row);
};

#pragma once

#include <optional>
#include <ostream>
#include <span>
#include <unordered_map>
#include <vector>

#include "../constants.h"
#include "../util/coordinate.h"
#include "piece.h"

namespace Game {

typedef std::optional<Piece> Tile;

std::ostream& operator<<(std::ostream& stream, const Tile& tile);

class Board {
 public:
  Board(const size_t width, const size_t height);
  Board(const std::vector<Tile>& grid, const size_t width, const size_t height);

  Board(const Board& other);
  Board(Board&& other);

  void Reset();
  void DoMove(const Coordinate source, const Offset offset);

  std::span<const Tile> GetGrid() const {
    return std::span<const Tile>(Grid.begin(), Width * Height);
  }
  std::optional<Tile> GetTile(const Coordinate coordinate) const;
  std::optional<std::span<const Tile>> GetRow(const size_t row) const;
  const std::vector<Coordinate>& GetPawnCoordinates(const Color color) const {
    return ColorToLocations.at(color);
  };
  const std::pair<size_t, size_t> GetDimensions() const {
    return {Width, Height};
  };
  bool MasterCaptured(Color color) const {
    return color == Color::Red ? RedMasterCaptured : BlueMasterCaptured;
  }

  bool OnBoard(const std::optional<const Coordinate> coordinate) const;
  std::optional<Color> IsFinished() const;

  std::ostream& StreamPlayer(std::ostream& stream, const Color player) const;
  std::ostream& StreamPlayerRow(std::ostream& stream, const Color player,
                                const size_t row, size_t& pawnIndex) const;

  friend std::ostream& operator<<(std::ostream& stream, const Board& board);

 private:
  std::vector<Tile> Grid;
  const size_t Width;
  const size_t Height;

  std::vector<Coordinate> RedLocations;
  bool RedMasterCaptured = false;
  std::vector<Coordinate> BlueLocations;
  bool BlueMasterCaptured = false;
  const std::unordered_map<Color, std::vector<Coordinate>&> ColorToLocations = {
      {Color::Red, RedLocations},
      {Color::Blue, BlueLocations},
  };

  void SetPieceCoordinates();

  size_t GetTileId(const Coordinate coordinate) const {
    return coordinate.y * Width + coordinate.x;
  }

  Tile GetTileUnchecked(const Coordinate coordinate) const {
    return Grid[GetTileId(coordinate)];
  }

  Tile& operator[](const Coordinate coordinate) {
    return Grid[GetTileId(coordinate)];
  };
  std::span<Tile> operator[](const size_t row);
};

std::ostream& operator<<(std::ostream& stream, const std::span<const Tile> row);

}  // namespace Game

#include "board.h"

#include "../util/ansiColor.h"

namespace Game {

static std::ostream& ColorPiece(std::ostream& stream, const Piece piece) {
  const AnsiColor::Foreground fgColor = AnsiColor::Foreground::White;
  const AnsiColor::Background bgColor = piece.Team == Color::Red
                                            ? AnsiColor::Background::Red
                                            : AnsiColor::Background::Blue;

  return stream << AnsiColor::Color(fgColor, bgColor);
}

std::ostream& operator<<(std::ostream& stream, const Tile& tile) {
  if (!tile.has_value()) return stream << '.';

  const char character =
      (tile->Master ? 'm' : 's') + ('A' - 'a') * (tile->Team == Color::Red);
  return ColorPiece(stream, *tile) << character << AnsiColor::Reset();
}

Board::Board() { Reset(); }

Board::Board(const std::array<Tile, BOARD_SIZE>& grid) : Grid(grid) {
  SetPieceCoordinates();
}

void Board::Reset() {
  constexpr size_t temple = BOARD_DIMENSIONS / 2;

  for (size_t x = 0; x < BOARD_DIMENSIONS; x++) {
    (*this)[0][x].emplace(TopPlayer, x == temple);
    (*this)[BOARD_DIMENSIONS - 1][x].emplace(~TopPlayer, x == temple);
    for (size_t y = 1; y < BOARD_DIMENSIONS - 1; y++) (*this)[y][x].reset();
  }

  SetPieceCoordinates();
}

bool Board::DoMove(const Coordinate source, const Offset offset) {
  // Check whether destination is on the board
  std::optional<Coordinate> destination = source.try_add(offset);
  if (!OnBoard(destination)) return false;

  // Check whether the source tile is a pawn
  Tile& srcTile = (*this)[source];
  if (!srcTile) return false;

  // Check whether the destination tile isn't a pawn of the same color
  Tile& destTile = (*this)[*destination];
  if (destTile && destTile->Team == srcTile->Team) return false;

  // Perform move
  if (destTile) destTile.reset();  // Capture
  srcTile.swap(destTile);

  SetPieceCoordinates();

  return true;
}

std::optional<Tile> Board::GetTile(const Coordinate coordinate) const {
  if (!OnBoard(coordinate)) return std::nullopt;

  return Grid[coordinate.y * BOARD_DIMENSIONS + coordinate.x];
}

std::optional<std::span<const Tile, BOARD_DIMENSIONS>> Board::GetRow(
    const size_t row) const {
  if (row >= BOARD_DIMENSIONS) return std::nullopt;

  return std::span<const Tile, BOARD_DIMENSIONS>(&Grid[row * BOARD_DIMENSIONS],
                                                 BOARD_DIMENSIONS);
}

std::vector<Coordinate> Board::GetPieceCoordinates(const Color color) const {
  switch (color) {
    case Color::Blue:
      return BlueLocations;
    case Color::Red:
      return RedLocations;

    default:
      size_t colorNum = (size_t)color;
      throw std::runtime_error(std::format("Invalid color {}", colorNum));
  }
}

void Board::SetPieceCoordinates() {
  RedLocations.clear();
  BlueLocations.clear();
  RedMasterCaptured = true;
  BlueMasterCaptured = true;

  size_t x = 0;
  size_t y = 0;

  for (const std::optional<Piece> tile : Grid) {
    if (tile) {
      std::vector<Coordinate>& locations =
          tile->Team == Color::Red ? RedLocations : BlueLocations;

      // Place master in the front
      if (tile->Master) {
        locations.insert(locations.begin(), Coordinate{x, y});

        if (tile->Team == Color::Red) {
          RedMasterCaptured = false;
        } else {
          BlueMasterCaptured = false;
        }

      } else {
        locations.emplace_back(x, y);
      }
    }

    x++;
    if (x == BOARD_DIMENSIONS) {
      x = 0;
      y++;
    }
  }
}

bool Board::OnBoard(const std::optional<const Coordinate> coordinate) const {
  return coordinate && coordinate->x < BOARD_DIMENSIONS &&
         coordinate->y < BOARD_DIMENSIONS;
}

std::optional<Color> Board::IsFinished() const {
  if (RedMasterCaptured) return Color::Blue;
  if (BlueMasterCaptured) return Color::Red;

  constexpr size_t temple = BOARD_DIMENSIONS / 2;
  if (RedLocations[0] == Coordinate(temple, BOARD_DIMENSIONS - 1))
    return Color::Red;
  if (BlueLocations[0] == Coordinate(temple, 0)) return Color::Blue;

  return std::nullopt;
}

Tile& Board::operator[](const Coordinate coordinate) {
  return Grid[coordinate.y * BOARD_DIMENSIONS + coordinate.x];
}

std::span<Tile, BOARD_DIMENSIONS> Board::operator[](const size_t row) {
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

std::ostream& Board::StreamPlayerRow(std::ostream& stream, const Color player,
                                     const size_t row,
                                     size_t& pawnIndex) const {
  for (const Tile& tile : *GetRow(row)) {
    if (tile && tile->Team == player && !tile->Master) {
      ColorPiece(stream, *tile) << ++pawnIndex << AnsiColor::Reset();
    } else {
      stream << tile;
    }
  }

  return stream;
}

std::ostream& Board::StreamPlayer(std::ostream& stream,
                                  const Color player) const {
  stream << std::endl;
  size_t pawnIndex = 0;

  for (size_t row = 0; row < BOARD_DIMENSIONS; row++) {
    StreamPlayerRow(stream, player, row, pawnIndex);
    stream << std::endl;
  }

  return stream;
}

}  // namespace Game

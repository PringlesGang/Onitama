#include "board.h"

#include <algorithm>
#include <cassert>
#include <format>

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

Board::Board(const size_t width, const size_t height)
    : Width(width), Height(height), Grid(width * height) {
  Reset();
}

Board::Board(const std::vector<Tile>& grid, const size_t width,
             const size_t height)
    : Grid(grid), Width(width), Height(height) {
  assert(Width * Height == grid.size());
  SetPieceCoordinates();
}

Board::Board(const Board& other)
    : Grid(other.Grid),
      Width(other.Width),
      Height(other.Height),
      RedLocations(other.RedLocations),
      BlueLocations(other.BlueLocations),
      RedMasterCaptured(other.RedMasterCaptured),
      BlueMasterCaptured(other.BlueMasterCaptured) {}

Board::Board(Board&& other)
    : Grid(std::move(other.Grid)),
      Width(std::move(other.Width)),
      Height(std::move(other.Height)),
      RedLocations(std::move(other.RedLocations)),
      BlueLocations(std::move(other.BlueLocations)),
      RedMasterCaptured(std::move(other.RedMasterCaptured)),
      BlueMasterCaptured(std::move(other.BlueMasterCaptured)) {}

void Board::Reset() {
  RedLocations.clear();
  BlueLocations.clear();
  RedMasterCaptured = false;
  BlueMasterCaptured = false;

  const size_t temple = Width / 2;

  std::vector<Coordinate>& topLocations = ColorToLocations.at(TopPlayer);
  std::vector<Coordinate>& bottomLocations = ColorToLocations.at(~TopPlayer);

  topLocations.emplace_back(Coordinate{temple, 0});
  bottomLocations.emplace_back(Coordinate{temple, Height - 1});

  for (size_t x = 0; x < Width; x++) {
    Grid[GetTileId(Coordinate{x, 0})].emplace(TopPlayer, x == temple);
    Grid[GetTileId(Coordinate{x, Height - 1})].emplace(~TopPlayer, x == temple);

    if (x != temple) {
      topLocations.emplace_back(Coordinate{x, 0});
      bottomLocations.emplace_back(Coordinate{x, Height - 1});
    }

    for (size_t y = 1; y < Height - 1; y++)
      Grid[GetTileId(Coordinate{x, y})].reset();
  }
}

void Board::DoMove(const Coordinate source, const Offset offset) {
  const Coordinate destination = source.try_add(offset).value();

  Tile& srcTile = Grid[GetTileId(source)];
  Tile& destTile = Grid[GetTileId(destination)];

  if (destTile) destTile.reset();  // Capture
  srcTile.swap(destTile);

  SetPieceCoordinates();
}

std::optional<Tile> Board::GetTile(const Coordinate coordinate) const {
  if (!OnBoard(coordinate)) return std::nullopt;

  return GetTileUnchecked(coordinate);
}

std::optional<std::span<const Tile>> Board::GetRow(const size_t row) const {
  if (row >= Height) return std::nullopt;

  return std::span<const Tile>(&Grid[row * Width], Width);
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
      std::vector<Coordinate>& locations = ColorToLocations.at(tile->Team);

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
    if (x == Width) {
      x = 0;
      y++;
    }
  }
}

bool Board::OnBoard(const std::optional<const Coordinate> coordinate) const {
  return coordinate && coordinate->x < Width && coordinate->y < Height;
}

std::optional<Color> Board::IsFinished() const {
  if (RedMasterCaptured) return Color::Blue;
  if (BlueMasterCaptured) return Color::Red;

  const size_t temple = Width / 2;
  if (ColorToLocations.at(TopPlayer)[0] == Coordinate{temple, Height - 1})
    return TopPlayer;
  if (ColorToLocations.at(~TopPlayer)[0] == Coordinate{temple, 0})
    return ~TopPlayer;

  return std::nullopt;
}

std::span<Tile> Board::operator[](const size_t row) {
  return std::span<Tile>(&Grid[row * Width], Width);
}

std::ostream& operator<<(std::ostream& stream,
                         const std::span<const Tile> row) {
  for (const Tile& tile : row) stream << tile;
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Board& board) {
  stream << std::endl;
  for (size_t row = 0; row < board.Height; row++) {
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

  for (size_t row = 0; row < Height; row++) {
    StreamPlayerRow(stream, player, row, pawnIndex);
    stream << std::endl;
  }

  return stream;
}

}  // namespace Game

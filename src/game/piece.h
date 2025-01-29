#pragma once

enum class Color {
  Red,
  Blue,
};

constexpr Color TopPlayer = Color::Red;

Color operator~(const Color& orig);

class Piece {
 public:
  Piece(Color color, bool master);
  Piece(const Piece& piece);
  Piece(Piece&& piece);

  void operator=(const Piece& other);

  Color GetColor() const { return Team; }
  bool IsMaster() const { return Master; }

 private:
  Color Team;
  bool Master;
};

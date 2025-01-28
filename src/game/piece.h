#pragma once

enum class Color {
  Red,
  Blue,
};

Color operator~(const Color& orig);

class Piece {
 public:
  Piece(Color color, bool master);
  Piece(const Piece& piece);
  Piece(Piece&& piece);

  void operator=(const Piece& other);

  Color GetColor() const { return Team; }
  bool IsMaster() const { return Master; }

  void Swap(Piece& other);

 private:
  Color Team;
  bool Master;
};

void swap(Piece& first, Piece& second);

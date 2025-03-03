#include "card.h"

#include <algorithm>

#include "../util/ansiColor.h"

namespace Game {

bool Card::HasMove(const Offset offset) const {
  const std::vector moves = GetMoves();
  return std::find(moves.begin(), moves.end(), offset) != moves.end();
}

std::ostream& Card::StreamRow(std::ostream& stream, const int8_t row,
                              const bool rotate) const {
  const std::vector<Offset>& offsets = GetMoves();
  const int8_t sign = rotate ? -1 : 1;

  for (Offset offset{.dx = (int8_t)(-2 * sign), .dy = row}; abs(offset.dx) <= 2;
       offset.dx += sign) {
    const auto offsetId = std::find(offsets.begin(), offsets.end(), offset);

    if (offsetId != offsets.end()) {
      stream << AnsiColor::Color(AnsiColor::Foreground::White,
                                 AnsiColor::Background::Gray)
             << std::distance(offsets.begin(), offsetId) << AnsiColor::Reset();
    } else if (offset.dx == 0 && row == 0) {
      stream << AnsiColor::Invert() << ' ' << AnsiColor::Reset();
    } else {
      stream << '.';
    }
  }

  return stream;
}

std::ostream& Card::Stream(std::ostream& stream, const bool rotate) const {
  const int8_t sign = rotate ? -1 : 1;

  stream << std::endl;
  for (int8_t row = -2 * sign; abs(row) <= 2; row++) {
    StreamRow(stream, row, rotate) << std::endl;
  }

  return stream;
}

}  // namespace Game

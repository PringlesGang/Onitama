#include "card.h"

#include <algorithm>
#include <format>
#include <iostream>

#include "../util/ansiColor.h"
#include "../util/parse.h"

namespace Game {

std::optional<Card> Card::Parse(std::istringstream& stream, bool fatal) {
  std::string input;
  stream >> input;
  Parse::ToLower(input);

  if (input.empty()) {
    if (fatal) std::cerr << "Tried to parse empty string as card!" << std::endl;

    return std::nullopt;
  }

  using namespace Game;

  if (input == "boar") return Card(CardType::Boar);
  if (input == "cobra") return Card(CardType::Cobra);
  if (input == "crab") return Card(CardType::Crab);
  if (input == "crane") return Card(CardType::Crane);
  if (input == "dragon") return Card(CardType::Dragon);
  if (input == "eel") return Card(CardType::Eel);
  if (input == "elephant") return Card(CardType::Elephant);
  if (input == "frog") return Card(CardType::Frog);
  if (input == "goose") return Card(CardType::Goose);
  if (input == "horse") return Card(CardType::Horse);
  if (input == "mantis") return Card(CardType::Mantis);
  if (input == "monkey") return Card(CardType::Monkey);
  if (input == "ox") return Card(CardType::Ox);
  if (input == "rabbit") return Card(CardType::Rabbit);
  if (input == "rooster") return Card(CardType::Rooster);
  if (input == "tiger") return Card(CardType::Tiger);

  if (fatal) {
    std::cerr << std::format("Invalid card name \"{}\"!", input) << std::endl;
  } else {
    Parse::Unparse(stream, input);
  }

  return std::nullopt;
}

bool Card::HasMove(const Offset offset) const {
  const std::vector moves = GetMoves();
  return std::find(moves.begin(), moves.end(), offset) != moves.end();
}

std::ostream& Card::StreamRow(std::ostream& stream, const int8_t row,
                              const bool rotate) const {
  const std::vector<Offset>& offsets = GetMoves();
  const int8_t sign = rotate ? -1 : 1;

  const int min = -(int)(CARD_DISPLAY_SIZE / 2);
  const int max = CARD_DISPLAY_SIZE + min;

  for (Offset offset{.dx = min * sign, .dy = row}; abs(offset.dx) < max;
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

  const int min = -(int)(CARD_DISPLAY_SIZE / 2);
  const int max = CARD_DISPLAY_SIZE + min;

  stream << std::endl;
  for (int row = min * sign; abs(row) < max; row++) {
    StreamRow(stream, row, rotate) << std::endl;
  }

  return stream;
}

}  // namespace Game

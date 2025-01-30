#include "color.h"

#include <format>
#include <stdexcept>
#include <string>

#include "ansiColor.h"

Color operator~(const Color& orig) {
  return orig == Color::Red ? Color::Blue : Color::Red;
}

std::ostream& operator<<(std::ostream& stream, Color color) {
  switch (color) {
    case Color::Blue:
      return stream << AnsiColor::Color(AnsiColor::Foreground::Blue) << "Blue"
                    << AnsiColor::Reset();
    case Color::Red:
      return stream << AnsiColor::Color(AnsiColor::Foreground::Red) << "Red"
                    << AnsiColor::Reset();

    default:
      size_t colorNum = (size_t)color;
      throw std::runtime_error(
          std::vformat("Invalid color {}", std::make_format_args(colorNum)));
  }
}

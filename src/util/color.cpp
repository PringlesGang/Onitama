#include "color.h"

#include <format>
#include <stdexcept>
#include <string>

Color operator~(const Color& orig) {
  return orig == Color::Red ? Color::Blue : Color::Red;
}

std::ostream& operator<<(std::ostream& stream, Color color) {
  switch (color) {
    case Color::Blue:
      return stream << "Blue";
    case Color::Red:
      return stream << "Red";

    default:
      size_t colorNum = (size_t)color;
      throw std::runtime_error(
          std::vformat("Invalid color {}", std::make_format_args(colorNum)));
  }
}

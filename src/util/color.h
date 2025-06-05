#pragma once

#include <ostream>

enum class Color {
  Red,
  Blue,
};

Color operator~(const Color& orig);

std::ostream& operator<<(std::ostream& stream, const Color color);

inline std::string to_string(Color color) {
  return color == Color::Red ? "red" : "blue";
}

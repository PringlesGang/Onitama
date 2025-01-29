#pragma once

#include <ostream>

enum class Color {
  Red,
  Blue,
};

constexpr Color TopPlayer = Color::Red;

Color operator~(const Color& orig);

std::ostream& operator<<(std::ostream& stream, Color color);

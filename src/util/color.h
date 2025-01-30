#pragma once

#include <ostream>

enum class Color {
  Red,
  Blue,
};

Color operator~(const Color& orig);

std::ostream& operator<<(std::ostream& stream, const Color color);

#pragma once

enum class Color {
  Red,
  Blue,
};

constexpr Color TopPlayer = Color::Red;

Color operator~(const Color& orig);

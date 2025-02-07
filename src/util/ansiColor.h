#pragma once

#include <string>

namespace AnsiColor {

enum class Background {
  Default = 49,
  Red = 41,
  Blue = 44,
  Gray = 100,
};

enum class Foreground {
  Default = 39,
  Black = 30,
  Red = 31,
  Blue = 34,
  White = 97,
};

std::string Color(const Foreground fg,
                  const Background bg = Background::Default);
inline std::string Reset() { return "\033[0m"; }
inline std::string Invert() { return "\033[7m"; }

}  // namespace AnsiColor

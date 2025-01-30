#include "ansiColor.h"

#include <format>

namespace AnsiColor {

std::string Color(Foreground fg, Background bg) {
  const size_t fgCode = (size_t)fg;
  const size_t bgCode = (size_t)bg;

  return std::format("\033[{};{}m", fgCode, bgCode);
}

};  // namespace AnsiColor

#include "winState.h"

#include <cstdint>

WinState operator-(const WinState original) {
  return WinState(-(int8_t)original);
}

bool operator<(const WinState lhs, const WinState rhs) {
  return (int8_t)lhs < (int8_t)rhs;
}

std::string to_string(WinState state) {
  switch (state) {
    case WinState::Win:
      return "Win";
    case WinState::Draw:
      return "Draw";
    case WinState::Lose:
      return "Lose";

    default:
      std::string msg = std::format("Unknown win state \"{}\"", (int8_t)state);
      throw std::runtime_error(msg);
  }
}

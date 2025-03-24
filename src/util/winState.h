#pragma once

enum class WinState {
  Lose = -1,
  Draw = 0,
  Win = 1,
};

WinState operator-(const WinState original);
bool operator<(const WinState lhs, const WinState rhs);

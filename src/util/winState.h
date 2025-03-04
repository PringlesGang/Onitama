#pragma once

enum class WinState {
  Lost = -1,
  Unknown = 0,
  Won = 1,

};

WinState operator-(const WinState original);
bool operator<(const WinState lhs, const WinState rhs);

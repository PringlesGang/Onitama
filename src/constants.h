#pragma once

#include "util/color.h"

constexpr size_t BOARD_DIMENSIONS = 5;
constexpr size_t BOARD_SIZE = BOARD_DIMENSIONS * BOARD_DIMENSIONS;

constexpr size_t HAND_SIZE = 2;
constexpr size_t CARD_COUNT = HAND_SIZE * 2 + 1;

constexpr Color TopPlayer = Color::Red;

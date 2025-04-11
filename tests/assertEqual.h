#pragma once

#include <span>

#include "../src/game/card.h"
#include "../src/game/game.h"

namespace Tests {

inline constexpr int Pass = 0;
inline constexpr int Fail = 1;

bool AssertEqual(const Color player, const Color expected);

bool AssertEqual(const ::Game::Card card, const ::Game::Card expected);
bool AssertEqual(const std::span<const ::Game::Card, HAND_SIZE> hand,
                 const std::span<const ::Game::Card, HAND_SIZE> expected);
bool AssertEqual(const std::span<const ::Game::Card, CARD_COUNT> cards,
                 const std::span<const ::Game::Card, CARD_COUNT> expected);

bool AssertEqual(const ::Game::Board& board, const ::Game::Board& expected);
bool AssertEqual(const ::Game::Game& game, const ::Game::Game& expected);

}  // namespace Tests

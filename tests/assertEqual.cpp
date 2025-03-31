#include "assertEqual.h"

#include <format>
#include <iostream>

namespace Tests {

bool AssertEqual(const Color player, const Color expected) {
  if (player != expected) {
    std::cerr << "Expected player " << expected << "; got " << player << "!"
              << std::endl;
    return false;
  }

  return true;
}

bool AssertEqual(const Game::Card card, const Game::Card expected) {
  if (card != expected) {
    std::cerr << std::format("Expected card {}; got {}!", expected.GetName(),
                             card.GetName())
              << std::endl;
    return false;
  }

  return true;
}

bool AssertEqual(const std::span<const Game::Card, HAND_SIZE> hand,
                 const std::span<const Game::Card, HAND_SIZE> expected) {
  if (!std::is_permutation(hand.begin(), hand.end(), expected.begin())) {
    std::cerr << "Hand {";
    for (Game::Card card : hand)
      std::cerr << std::format("{}, ", card.GetName());
    std::cerr << "} is not equal to expected {";
    for (Game::Card card : expected)
      std::cerr << std::format("{}, ", card.GetName());
    std::cerr << "}!" << std::endl;

    return false;
  }

  return true;
}

bool AssertEqual(const std::span<const Game::Card, CARD_COUNT> cards,
                 const std::span<const Game::Card, CARD_COUNT> expected) {
  if (cards[0] != expected[0]) {
    std::cerr << std::format("Set-aside card not equal! Expected {}, got {}",
                             expected[0].GetName(), cards[0].GetName())
              << std::endl;
    return false;
  }

  for (size_t playerNum = 0; playerNum < 2; playerNum++) {
    const size_t offset = 1 + playerNum * HAND_SIZE;
    if (!AssertEqual(std::span<const Game::Card, HAND_SIZE>(
                         cards.subspan(offset, HAND_SIZE)),
                     std::span<const Game::Card, HAND_SIZE>(
                         expected.subspan(offset, HAND_SIZE)))) {
      return false;
    }
  }

  return true;
}

bool AssertEqual(const Game::Board& board, const Game::Board& expected) {
  if (board != expected) {
    std::cerr << "Boards are not equal!\nExpected" << expected << "Got:\n"
              << board << std::endl;
    return false;
  }

  return true;
}

bool AssertEqual(const Game::Game& game, const Game::Game& expected) {
  return AssertEqual(game.GetBoard(), expected.GetBoard()) &&
         AssertEqual(game.GetCards(), expected.GetCards()) &&
         AssertEqual(game.GetCurrentPlayer(), expected.GetCurrentPlayer());
}

}  // namespace Tests

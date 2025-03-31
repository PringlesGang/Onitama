#include "game.h"

#include <format>
#include <iostream>

#include "../../src/game/game.h"

namespace Tests {
namespace Game {

using namespace ::Game;

int InitialStateConstructor() {
  constexpr size_t width = 3;
  constexpr size_t height = 4;

  constexpr std::array<Card, CARD_COUNT> cards = {
      Card{CardType::Crab}, Card{CardType::Mantis}, Card{CardType::Cobra},
      Card{CardType::Rooster}, Card{CardType::Cobra}};

  Game::Game game = Game::Game(3, 4, cards);

  // Check dimensions
  const std::pair<size_t, size_t> dimensions = game.GetBoard().GetDimensions();
  if (game.GetBoard().GetDimensions() != std::pair(width, height)) {
    std::cerr << std::format("Expected dimensions {}x{}, got {}x{}!", width,
                             height, dimensions.first, dimensions.second)
              << std::endl;
    return 1;
  }

  // Check cards
  if (!std::equal(cards.begin(), cards.end(), game.GetCards().begin())) {
    std::cerr << "Invalid card configuration!\nExpected: ";
    for (const Card card : cards) {
      std::cerr << std::format("{}, ", card.GetName());
    }
    std::cerr << "\nGot: ";
    for (const Card card : game.GetCards()) {
      std::cerr << std::format("{}, ", card.GetName());
    }
    std::cerr << std::endl;

    return 1;
  }

  // Check board layout
  if (game.GetBoard() != Board(width, height)) {
    std::cerr << "Boards are not equal!\nExpected" << Board(width, height)
              << "Got:\n"
              << game.GetBoard() << std::endl;
    return 1;
  }

  // Check initial player
  if (game.GetCurrentPlayer() != cards[0].GetColor()) {
    std::cerr << "Expected initial player " << cards[0].GetColor() << " got "
              << game.GetCurrentPlayer() << "!" << std::endl;
    return 1;
  }

  return 0;
}

}  // namespace Game
}  // namespace Tests

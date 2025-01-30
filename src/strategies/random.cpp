#include "random.h"

#include <random>

namespace Strategy {

Game::Move Random::GetMove(const Game::Game& game) {
  const std::unordered_set validMoves = game.GetValidMoves();
  const size_t moveCount = validMoves.empty() ? HAND_SIZE : validMoves.size();

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<size_t> randomMove(0, moveCount - 1);

  size_t randomMoveIndex = randomMove(generator);

  if (validMoves.empty()) {
    return Game::Move{.PawnId = 0,
                      .UsedCard = game.GetCurrentHand()[randomMoveIndex],
                      .OffsetId = 0};
  } else {
    auto move = validMoves.begin();
    for (size_t i = 0; i < randomMoveIndex; i++) ++move;
    return *move;
  }
}

}  // namespace Strategy
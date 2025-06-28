#include "random.h"

namespace Strategy {

Random::Random() : Generator(RandomDevice()) {}

Game::Move Random::GetMove(const Game::Game& game) {
  const std::vector<Game::Move>& validMoves = game.GetValidMoves();

  std::uniform_int_distribution<size_t> randomMove(0, validMoves.size() - 1);

  return validMoves[randomMove(Generator)];
}

std::optional<std::function<std::unique_ptr<Random>()>> Random::Parse(
    std::istringstream& stream) {
  return std::make_unique<Random>;
}

}  // namespace Strategy
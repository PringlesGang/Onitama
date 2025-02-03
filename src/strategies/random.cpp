#include "random.h"

namespace Strategy {

Random::Random() : Generator(RandomDevice()) {}

Game::Move Random::GetMove(const Game::Game& game) {
  const std::unordered_set validMoves = game.GetValidMoves();
  const size_t moveCount = validMoves.empty() ? HAND_SIZE : validMoves.size();

  std::uniform_int_distribution<size_t> randomMove(0, moveCount - 1);
  size_t randomMoveIndex = randomMove(Generator);

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

std::optional<std::function<std::unique_ptr<Random>()>> Random::Parse(
    std::istringstream& command) {
  return std::make_unique<Random>;
}

std::string Random::GetName() { return "random"; }

std::string Random::GetCommand() { return GetName(); }

std::string Random::GetDescription() {
  return "Performs a uniformly distributed random move out of all valid ones.";
}

}  // namespace Strategy
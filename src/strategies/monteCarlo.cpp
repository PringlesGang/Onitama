#include "monteCarlo.h"

#include <format>
#include <iostream>

namespace Strategy {

MonteCarlo::MonteCarlo(size_t repeatCount) : RepeatCount(repeatCount) {}

Game::Move MonteCarlo::GetMove(const Game::Game& game) {
  std::unordered_set<Game::Move> validMoves = game.GetValidMoves();

  if (validMoves.empty()) {
    for (Game::Card card : game.GetCurrentHand()) {
      validMoves.insert(Game::Move{
          .PawnId = 0,
          .UsedCard = card,
          .OffsetId = 0,
      });
    }
  }

  Game::Move bestMove;
  size_t bestMoveWinCount = 0;

  for (Game::Move move : validMoves) {
    size_t winCount = 0;

    for (size_t i = 0; i < RepeatCount; i++) {
      Game::Game nextState = Game::Game(game);
      if (!nextState.DoMove(move)) continue;

      winCount += RunSimulation(nextState) == game.GetCurrentPlayer();
    }

    if (winCount > bestMoveWinCount) {
      bestMove = move;
      bestMoveWinCount = winCount;
    }

    if (winCount == RepeatCount) break;  // Cannot improve upon this
  }

  return bestMove;
}

Color MonteCarlo::RunSimulation(Game::Game& game) {
  while (!game.IsFinished()) {
    game.DoMove(RandomStrategy.GetMove(game));
  }

  return game.IsFinished().value();
}

std::optional<std::function<std::unique_ptr<MonteCarlo>()>> MonteCarlo::Parse(
    std::istringstream& command) {
  size_t repeatCount;
  if (!(command >> repeatCount)) {
    std::cout << "Did not provide valid repeat count for Monte Carlo strategy!"
              << std::endl;
    return std::nullopt;
  }

  if (repeatCount == 0) {
    std::cout << "Monte Carlo repeat count must be greater than 0!"
              << std::endl;
    return std::nullopt;
  }

  return [repeatCount] { return std::make_unique<MonteCarlo>(repeatCount); };
}

std::string MonteCarlo::GetName() { return "montecarlo"; }

std::string MonteCarlo::GetCommand() {
  return std::format("{} repeat_count", GetName());
}

std::string MonteCarlo::GetDescription() {
  return "Performs a random simulation for a specified amount of times "
         "and picks the move that lead to the most wins.";
}

}  // namespace Strategy

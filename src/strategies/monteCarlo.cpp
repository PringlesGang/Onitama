#include "monteCarlo.h"

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
  }

  return bestMove;
}

Color MonteCarlo::RunSimulation(Game::Game& game) {
  while (!game.IsFinished()) {
    game.DoMove(RandomStrategy.GetMove(game));
  }

  return game.IsFinished().value();
}

}  // namespace Strategy

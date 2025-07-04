#include "minMax.h"

#include <format>
#include <future>
#include <iostream>
#include <sstream>

#include "../cli/command.h"

namespace Strategy {

MinMax::MinMax(const std::optional<const size_t> maxDepth)
    : MaxDepth(maxDepth) {}

Game::Move MinMax::GetMove(const Game::Game& game) {
  const std::vector<Game::Move>& moves = game.GetValidMoves();

  std::vector<std::pair<Game::Move, std::future<WinState>>> futures;
  futures.reserve(moves.size());

  for (Game::Move move : moves) {
    Game::Game nextState = Game::Game(game);
    nextState.DoMove(move);

    futures.emplace_back(
        move, std::async(std::launch::async, &MinMax::PlayRecursive, this,
                         std::move(nextState), 0));
  }

  WinState bestMoveValue = WinState::Lose;
  Game::Move bestMove = moves[0];

  for (auto& [move, future] : futures) {
    const WinState value = -future.get();

    if (value > bestMoveValue) {
      bestMoveValue = value;
      bestMove = move;
    }
  }

  return bestMove;
}

WinState MinMax::PlayRecursive(Game::Game game, const size_t depth) const {
  if (depth == MaxDepth) return WinState::Draw;

  const std::optional<Color> winner = game.IsFinished();
  if (winner) {
    return winner.value() == game.GetCurrentPlayer() ? WinState::Win
                                                     : WinState::Lose;
  }

  const std::vector<Game::Move>& moves = game.GetValidMoves();

  WinState best = WinState::Lose;
  for (const Game::Move move : moves) {
    Game::Game nextState = Game::Game(game);
    nextState.DoMove(move);

    best = std::max(-PlayRecursive(std::move(nextState), depth + 1), best);
  }

  return best;
}

std::optional<std::function<std::unique_ptr<MinMax>()>> MinMax::Parse(
    std::istringstream& stream) {
  std::string argument;
  if (!(stream >> argument)) {
    std::cout << "Failed to parse MinMax strategy argument!" << std::endl;
    return std::nullopt;
  }
  Parse::ToLower(argument);

  std::optional<size_t> maxDepth;
  if (argument == "--no-max-depth") {
    maxDepth = std::nullopt;
  } else {
    try {
      maxDepth = std::stoull(argument);
    } catch (std::invalid_argument err) {
      std::cout << std::format(
                       "Failed to parse max depth {} for MinMax strategy!",
                       argument)
                << std::endl;
      return std::nullopt;
    }
  }

  return [maxDepth] { return std::make_unique<MinMax>(maxDepth); };
}

}  // namespace Strategy
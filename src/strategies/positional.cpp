#include "positional.h"

namespace Strategy {

GameStateInfo::GameStateInfo(const Game::Game& game)
    : Serialization(game.Serialize()),
      Quality(game.IsFinished() ? WinState::Lost : WinState::Unknown) {}

std::optional<std::weak_ptr<const GameStateInfo>> GameStateGraph::Get(
    const Game::Game& game) const {
  const Game::GameSerialization serialization = game.Serialize();
  return Vertices.contains(game)
             ? std::optional(std::weak_ptr(Vertices.at(game)))
             : std::nullopt;
};

std::weak_ptr<const GameStateInfo> GameStateGraph::Add(Game::Game&& game) {
  const std::optional<std::weak_ptr<const GameStateInfo>> storedInfo =
      Get(game);
  if (storedInfo) return storedInfo.value();

  std::shared_ptr<GameStateInfo> info = std::make_shared<GameStateInfo>(game);
  Vertices.emplace(game, info);

  // Terminal game state
  if (info->Quality != WinState::Unknown) return info;

  const std::vector<Game::Move>& validMoves = game.GetValidMoves();
  for (const Game::Move move : validMoves) {
    Game::Game nextState(game);
    nextState.DoMove(move);

    // Traverse further
    std::shared_ptr<const GameStateInfo> nextInfo =
        Add(std::move(nextState)).lock();
    const WinState nextQuality = -nextInfo->Quality;

    // Select the best move yet
    if (!info->OptimalMove || info->Quality < nextQuality) {
      info->Quality = nextQuality;
      info->OptimalMove =
          std::pair<Game::Move, std::weak_ptr<const GameStateInfo>>(move,
                                                                    nextInfo);
    }

    // A winning positional strategy has been found
    if (info->Quality == WinState::Won) return info;
  }

  return info;
}

Positional::Positional() : Graph(SharedGameStateGraph) {}

Positional::Positional(std::shared_ptr<GameStateGraph> graph) : Graph(graph) {}

Game::Move Positional::GetMove(const Game::Game& game) {
  const std::optional<std::weak_ptr<const GameStateInfo>> found =
      Graph->Get(game);

  // Either get the pre-computed positional strategy,
  // or compute the positional strategy now
  return found ? found->lock()->GetOptimalMove()
               : Graph->Add(Game::Game(game)).lock()->GetOptimalMove();
}

std::optional<std::function<std::unique_ptr<Positional>()>> Positional::Parse(
    std::istringstream& command) {
  return std::make_unique<Positional>;
}

std::string Positional::GetName() { return "positional"; }

std::string Positional::GetCommand() { return GetName(); }

std::string Positional::GetDescription() {
  return "Slowly seeks out a perfect positional strategy to perform.";
}

}  // namespace Strategy

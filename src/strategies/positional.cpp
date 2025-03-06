#include "positional.h"

#include <unordered_set>

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

static bool CompareCoordinates(const Game::Game& first,
                               const Game::Game& second,
                               const Color firstPlayer,
                               const Color secondPlayer) {
  const bool masterCaptured = first.MasterCaptured(firstPlayer);
  if (masterCaptured != second.MasterCaptured(secondPlayer)) return false;

  const bool orient = firstPlayer != secondPlayer;
  const auto [width, height] = second.GetDimensions();
  const auto orientCoord = [orient, width, height](Coordinate coordinate) {
    return orient
               ? Coordinate(width - coordinate.x - 1, height - coordinate.y - 1)
               : coordinate;
  };

  const std::vector<Coordinate>& firstCoordinates =
      first.GetPawnCoordinates(firstPlayer);
  const std::vector<Coordinate>& secondCoordinates =
      second.GetPawnCoordinates(secondPlayer);

  if (firstCoordinates.size() != secondCoordinates.size()) return false;
  if (firstCoordinates.size() == 0) return true;

  if (!masterCaptured &&
      firstCoordinates[0] != orientCoord(secondCoordinates[0]))
    return false;

  auto secondCoord = orient ? --secondCoordinates.end()
                            : secondCoordinates.begin() + !masterCaptured;
  for (auto firstCoord = firstCoordinates.begin() + !masterCaptured;
       firstCoord != firstCoordinates.end(); firstCoord++) {
    if (*firstCoord != orientCoord(*secondCoord)) return false;

    if (!orient || secondCoord != secondCoordinates.begin())
      secondCoord += orient ? -1 : 1;
  }

  return true;
}

bool GameVertexEqual::operator()(const Game::Game& first,
                                 const Game::Game& second) const {
  if (first.GetSetAsideCard() != second.GetSetAsideCard()) return false;

  if (first.GetDimensions() != second.GetDimensions()) return false;

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color firstPlayer =
        playerId == 0 ? first.GetCurrentPlayer() : ~first.GetCurrentPlayer();
    const Color secondPlayer =
        playerId == 0 ? second.GetCurrentPlayer() : ~second.GetCurrentPlayer();

    const std::span<const Game::Card, HAND_SIZE> firstHand =
        first.GetHand(firstPlayer);
    const std::span<const Game::Card, HAND_SIZE> secondHand =
        first.GetHand(firstPlayer);
    if (std::unordered_multiset<Game::Card>(firstHand.begin(),
                                            firstHand.end()) !=
        std::unordered_multiset<Game::Card>(secondHand.begin(),
                                            secondHand.end()))
      return false;

    if (!CompareCoordinates(first, second, firstPlayer, secondPlayer))
      return false;
  }

  return true;
}

size_t GameVertexHash::operator()(const Game::Game& game) const {
  // Cards
  size_t hash = (size_t)game.GetSetAsideCard().Type;

  for (const Game::Card card : game.GetHand(TopPlayer)) {
    hash ^= (size_t)card.Type;
  }

  // Pawn locations
  const auto [width, height] = game.GetDimensions();
  const size_t boardEnd = width * height - 1;

  const bool orient = TopPlayer == game.GetCurrentPlayer();

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : ~TopPlayer;
    for (const Coordinate coordinate : game.GetPawnCoordinates(player)) {
      const size_t offset = coordinate.x + width * coordinate.y;
      hash ^= orient ? offset : boardEnd - offset;
    }
  }

  return hash;
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

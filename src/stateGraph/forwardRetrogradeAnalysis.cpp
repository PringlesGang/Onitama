#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::ForwardRetrogradeAnalysis(
    Game::Game&& game) {
  // Reset clocks
  LastSaveTime = std::chrono::system_clock::now();
  StartingTime = LastSaveTime;

  std::shared_ptr<Vertex> rootVertex = std::make_shared<Vertex>(game);
  Vertices.emplace(game, rootVertex);

  std::unordered_set<std::shared_ptr<Vertex>> expandedVertices;
  std::unordered_set<std::shared_ptr<Edge>> unlabelledEdges;
  std::deque<Game::GameSerialization> callStack;
  bool reinstatingCallStack = false;
  ForwardRetrogradeAnalysisExpand(rootVertex, expandedVertices, unlabelledEdges,
                                  rootVertex, callStack, reinstatingCallStack);

  RetrogradeAnalyseEdges(unlabelledEdges);

  const auto isDraw = [&expandedVertices](std::shared_ptr<Edge> edge) -> bool {
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    if (target == nullptr) return false;

    return (target->Quality.value_or(WinState::Draw) == WinState::Draw) &&
           expandedVertices.contains(target);
  };

  // All edges between two expanded, non-labelled vertices result in draws
  for (const std::shared_ptr<Vertex> vertex : expandedVertices) {
    if (vertex->Quality.has_value()) continue;

    const auto drawMove =
        std::find_if(vertex->Edges.begin(), vertex->Edges.end(), isDraw);
    if (drawMove == vertex->Edges.end()) continue;

    vertex->SetOptimalMove((*drawMove)->Move);
    vertex->Quality = WinState::Draw;
  }

  PrintRunningTime();

  return rootVertex;
}

std::weak_ptr<const Vertex> Graph::ForwardRetrogradeAnalysis(
    ForwardRetrogradeProgress progress) {
  // Reset clocks
  LastSaveTime = std::chrono::system_clock::now();
  StartingTime = LastSaveTime - progress.Runtime;

  const std::shared_ptr<Vertex> root =
      Vertices.at(Game::Game::FromSerialization(progress.CallStack.front()));
  std::unordered_set<std::shared_ptr<Vertex>>& expandedVertices =
      progress.ExpandedVertices;
  bool reinstateCallStack = true;

  ForwardRetrogradeAnalysisExpand(root, progress.ExpandedVertices,
                                  progress.UnlabelledEdges, root,
                                  progress.CallStack, reinstateCallStack);

  RetrogradeAnalyseEdges(progress.UnlabelledEdges);

  const auto isDraw = [&expandedVertices](std::shared_ptr<Edge> edge) -> bool {
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    if (target == nullptr) return false;

    return (target->Quality.value_or(WinState::Draw) == WinState::Draw) &&
           expandedVertices.contains(target);
  };

  // All edges between two expanded, non-labelled vertices result in draws
  for (const std::shared_ptr<Vertex> vertex : expandedVertices) {
    if (vertex->Quality.has_value()) continue;

    const auto drawMove =
        std::find_if(vertex->Edges.begin(), vertex->Edges.end(), isDraw);
    if (drawMove == vertex->Edges.end()) continue;

    vertex->SetOptimalMove((*drawMove)->Move);
    vertex->Quality = WinState::Draw;
  }

  PrintRunningTime();

  return root;
}

void Graph::ForwardRetrogradeAnalysisExpand(
    const std::shared_ptr<Vertex> source,
    std::unordered_set<std::shared_ptr<Vertex>>& expandedVertices,
    std::unordered_set<std::shared_ptr<Edge>>& unlabelledEdges,
    const std::shared_ptr<const Vertex> root,
    std::deque<Game::GameSerialization>& callStack,
    bool& reinstatingCallStack) {
  if (!reinstatingCallStack) callStack.push_back(source->Serialization);

  if (expandedVertices.contains(source)) {
    if (!reinstatingCallStack) return;

    const bool inCallStack =
        std::find(callStack.begin(), callStack.end(), source->Serialization) !=
        callStack.end();
    if (!inCallStack) return;

    if (source->Serialization == callStack.back()) {
      reinstatingCallStack = false;
      return;
    }
  }
  expandedVertices.insert(source);

  const Game::Game game = Game::Game::FromSerialization(source->Serialization);

  // Terminal game state
  if (!reinstatingCallStack && source->Quality.has_value()) {
    RetrogradeAnalyseEdges(unlabelledEdges);

    if (IntermediatePath && std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::system_clock::now() - LastSaveTime)
                                    .count() >= SaveTimeInterval) {
      ForwardRetrogradeProgress progress{
          .ExpandedVertices = expandedVertices,
          .UnlabelledEdges = unlabelledEdges,
          .CallStack = callStack,
      };
      SaveForwardRetrogradeAnalysis(IntermediatePath.value(),
                                    std::move(progress));

      LastSaveTime = std::chrono::system_clock::now();
    }

    return;
  }

  // Insert edges
  if (!reinstatingCallStack) {
    const std::vector<Game::Move>& validMoves = game.GetValidMoves();
    for (const Game::Move move : validMoves) {
      Game::Game nextState(game);
      nextState.DoMove(move);

      if (!Vertices.contains(nextState))
        Vertices.emplace(nextState, std::make_shared<Vertex>(nextState));
      const std::shared_ptr<Vertex> target = Vertices.at(nextState);

      const std::shared_ptr<Edge> edge =
          std::make_shared<Edge>(source, target, move);
      unlabelledEdges.insert(edge);
      source->Edges.emplace_back(std::move(edge));
    }
  } else {
    const Game::GameSerialization nextState =
        *(std::find(callStack.begin(), callStack.end(), source->Serialization) +
          1);
    const std::shared_ptr<Edge> nextEdge = source->GetEdge(nextState).value();
    const std::shared_ptr<Vertex> nextVertex = nextEdge->Target.lock();

    ForwardRetrogradeAnalysisExpand(nextVertex, expandedVertices,
                                    unlabelledEdges, root, callStack,
                                    reinstatingCallStack);
    callStack.pop_back();
  }

  bool allLabelled = true;
  for (auto edgeIt = source->Edges.begin(); edgeIt != source->Edges.end();
       edgeIt++) {
    const std::shared_ptr<Edge> edge = *edgeIt;
    if (edge->Optimal.has_value()) continue;

    const bool finalEdge = edgeIt + 1 == source->Edges.end();

    std::shared_ptr<Vertex> target = edge->Target.lock();
    if (target == nullptr) {
      unlabelledEdges.erase(edge);
      continue;
    }

    ForwardRetrogradeAnalysisExpand(target, expandedVertices, unlabelledEdges,
                                    root, callStack, reinstatingCallStack);
    callStack.pop_back();

    // Optimal strategy from root has been established; exit algorithm
    if (root->Quality.has_value()) {
      // Current vertex was not fully expanded
      expandedVertices.erase(source);
      return;
    }

    if (!target->Quality.has_value()) {
      allLabelled = false;
      continue;
    }

    switch (target->Quality.value()) {
      case WinState::Lose: {
        source->Quality = WinState::Win;
        source->SetOptimalMove(edge->Move);
        unlabelledEdges.erase(edge);

        return;
      }

      case WinState::Win: {
        if (finalEdge && allLabelled) {  // last unlabelled
          source->Quality = WinState::Lose;
          source->SetOptimalMove(edge->Move);
        } else {
          edge->Optimal = false;
        }

        unlabelledEdges.erase(edge);
        continue;
      }

      default: {
        const std::string msg = std::format("Unexpected target quality \"{}\"!",
                                            (int8_t)target->Quality.value());
        throw std::runtime_error(msg);
      }
    }
  }
}

}  // namespace StateGraph

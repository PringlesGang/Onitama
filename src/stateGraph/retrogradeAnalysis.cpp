#include <cassert>
#include <iostream>

#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::RetrogradeAnalysis(Game::Game&& game) {
  std::cout << "Exploring state graph..." << std::endl;

  std::unordered_set<std::shared_ptr<Edge>> edges;
  std::unordered_set<Game::Game, Hash, EqualTo> explored;
  RetrogradeAnalysisExpand(Game::Game(game), explored, edges);

  std::cout << "Analysing edges..." << std::endl;
  RetrogradeAnalyseEdges(edges);

  // All unlabelled vertices are Draw
  for (auto [_, vertex] : Vertices) {
    if (vertex->Quality.has_value()) continue;

    vertex->Quality = WinState::Draw;
    for (std::shared_ptr<Edge> edge : vertex->Edges) {
      if (edge->Optimal.has_value()) continue;

      assert(edge->Target.lock()->Quality.value_or(WinState::Draw) ==
             WinState::Draw);

      vertex->SetOptimalMove(edge->Move);
    }
  }

  return Vertices.at(std::move(game));
}

std::weak_ptr<Vertex> Graph::RetrogradeAnalysisExpand(
    Game::Game&& game, std::unordered_set<Game::Game, Hash, EqualTo>& explored,
    std::unordered_set<std::shared_ptr<Edge>>& edges) {
  if (explored.contains(game)) return Vertices.at(std::move(game));

  std::shared_ptr<Vertex> vertex = std::make_shared<Vertex>(game);

  explored.insert(game);
  Vertices.insert({game, vertex});

  if (game.IsFinished()) {
    return vertex;
  }

  for (const Game::Move move : game.GetValidMoves()) {
    Game::Game nextGame = Game::Game(game);
    nextGame.DoMove(move);

    std::weak_ptr<Vertex> target =
        RetrogradeAnalysisExpand(std::move(nextGame), explored, edges);

    std::shared_ptr<Edge> edge = vertex->Edges.emplace_back(
        std::make_shared<Edge>(vertex, target, move));
    edges.emplace(edge);
  }

  return vertex;
}

void Graph::RetrogradeAnalyseEdges(
    std::unordered_set<std::shared_ptr<Edge>>& edges) {
  bool edgeLabelled = true;
  while (!edges.empty() && edgeLabelled) {
    edgeLabelled = false;

    auto edgeIt = edges.begin();
    while (edgeIt != edges.end()) {
      std::shared_ptr<Edge> edge = *edgeIt;

      // Cannot relabel an already labelled edge
      if (edge->Optimal.has_value()) {
        edgeIt = edges.erase(edgeIt);
        continue;
      }

      const std::shared_ptr<Vertex> source = edge->Source.lock();
      const std::shared_ptr<Vertex> target = edge->Target.lock();
      if (source == nullptr || target == nullptr) {
        edge->Optimal = false;
        edgeLabelled = true;
        edgeIt = edges.erase(edgeIt);
        continue;
      }

      // Cannot relabel an already labelled node
      if (source->Quality.has_value()) {
        edge->Optimal = false;
        edgeLabelled = true;
        edgeIt = edges.erase(edgeIt);
        continue;
      }

      // Target does not (yet) provide any information
      if (!target->Quality.has_value()) {
        edgeIt++;
        continue;
      }

      if (target->Quality.value() == WinState::Lose) {
        // This is the optimal move
        source->Quality = WinState::Win;
        source->SetOptimalMove(edge->Move);

        edgeLabelled = true;
        edgeIt = edges.erase(edgeIt);

      } else if (target->Quality.value() == WinState::Win) {
        // If it's the last unlabelled edge -> optimal and losing,
        // else -> redundant; keep looking
        const auto unlabelled = [edge](std::shared_ptr<Edge> itEdge) {
          return itEdge != edge && !itEdge->Optimal.has_value();
        };
        const bool lastUnlabelledEdge =
            std::find_if(source->Edges.begin(), source->Edges.end(),
                         unlabelled) == source->Edges.end();

        if (lastUnlabelledEdge) {
          source->Quality = WinState::Lose;
          source->SetOptimalMove(edge->Move);
        } else {
          edge->Optimal = false;
        }

        edgeLabelled = true;
        edgeIt = edges.erase(edgeIt);

      } else {
        const std::string msg = std::format("Unexpected target quality \"{}\"!",
                                            (int8_t)target->Quality.value());
        throw std::runtime_error(msg);
      }
    }
  }
}

}  // namespace StateGraph

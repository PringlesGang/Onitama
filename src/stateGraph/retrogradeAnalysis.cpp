#include <cassert>
#include <iostream>

#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::RetrogradeAnalysis(Game::Game&& game) {
  std::cout << "Exploring state graph..." << std::endl;
  StartingTime = std::chrono::system_clock::now();

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

  PrintRunningTime();

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

void RetrogradeAnalyse(Graph& graph) {
  // Keep trying until an uneventful loop occurred
  bool edgeLabelled = false;
  std::unordered_set<std::shared_ptr<Vertex>> unlabelledExpandedVertices;

  // Analyse wins and losses
  do {
    edgeLabelled = false;

    // Check all vertices
    for (auto& [game, vertex] : graph.Vertices) {
      // Cannot relabel a labelled vertex
      if (vertex->Quality.has_value()) continue;

      if (!vertex->Edges.empty()) unlabelledExpandedVertices.insert(vertex);

      // Check all edges
      for (auto edgeIt = vertex->Edges.begin(); edgeIt != vertex->Edges.end();
           edgeIt++) {
        const std::shared_ptr<Edge> edge = *edgeIt;

        // Cannot relabel a labelled edge
        if (edge->Optimal.has_value()) continue;

        // Cannot label an edge with an unlabelled target
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);
        if (!target->Quality.has_value()) continue;

        const bool lastUnlabelledEdge =
            std::all_of(vertex->Edges.begin(), vertex->Edges.end(),
                        [edge](std::shared_ptr<Edge> otherEdge) {
                          return edge->Move == otherEdge->Move ||
                                 otherEdge->Optimal.has_value();
                        });

        switch (target->Quality.value()) {
          case WinState::Lose: {
            vertex->Quality = WinState::Win;
            vertex->SetOptimalMove(edge->Move);
            edgeLabelled = true;
            break;
          }

          case WinState::Win: {
            if (lastUnlabelledEdge) {
              if (vertex->GetOptimalMove().has_value()) {
                // Draw move was already found
                vertex->Quality = WinState::Draw;
                edge->Optimal = false;
              } else {
                // No draw move; only losing moves
                vertex->Quality = WinState::Lose;
                vertex->SetOptimalMove(edge->Move);
              }

            } else {
              edge->Optimal = false;
            }

            edgeLabelled = true;
            break;
          }

          case WinState::Draw: {
            // Provisionally set the optimal move to draw
            edgeLabelled = true;
            vertex->SetOptimalMove(edge->Move);
            if (lastUnlabelledEdge) vertex->Quality = WinState::Draw;
            break;
          }

          default: {
            const std::string msg =
                std::format("Unexpected target quality \"{}\"!",
                            (int8_t)target->Quality.value());
            throw std::runtime_error(msg);
          }
        }

        // The vertex' quality has been determined
        if (vertex->Quality.has_value()) {
          unlabelledExpandedVertices.erase(vertex);
          break;
        }
      }
    }
  } while (edgeLabelled);

  // Assign draws
  bool anyVertexRemoved = false;
  bool currentVertexRemoved = false;
  const auto removeVertex =
      [&anyVertexRemoved, &currentVertexRemoved, &unlabelledExpandedVertices](
          std::unordered_set<std::shared_ptr<Vertex>>::iterator& vertex) {
        anyVertexRemoved = true;
        currentVertexRemoved = true;
        vertex = unlabelledExpandedVertices.erase(vertex);
      };

  do {
    anyVertexRemoved = false;

    for (auto vertexIt = unlabelledExpandedVertices.begin();
         vertexIt != unlabelledExpandedVertices.end();) {
      const std::shared_ptr<Vertex> vertex = *vertexIt;
      currentVertexRemoved = false;

      for (const std::shared_ptr<Edge> edge : vertex->Edges) {
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);

        if (target->Quality.has_value()) {
          if (target->Quality.value() == WinState::Draw) {
            // Take the guaranteed draw
            vertex->SetOptimalMove(edge->Move);
            vertex->Quality = WinState::Draw;

            removeVertex(vertexIt);
            break;
          }

          assert(target->Quality.value() != WinState::Lose);
          continue;
        }

        // There is an unexplored node; draw not necessarily optimal
        if (!unlabelledExpandedVertices.contains(target)) {
          removeVertex(vertexIt);
          break;
        }

        // Set the supposed draw move
        vertex->SetOptimalMove(edge->Move);
      }
      assert(currentVertexRemoved || vertex->GetOptimalMove().has_value());

      if (!currentVertexRemoved) vertexIt++;
    }
  } while (anyVertexRemoved);

  // Mark unlabelled expanded vertices with only expanded children as draws
  for (const std::shared_ptr<Vertex> draw : unlabelledExpandedVertices) {
    draw->Quality = WinState::Draw;
    assert(draw->GetOptimalMove().has_value());
  }
}

}  // namespace StateGraph

#include <cassert>
#include <iostream>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {

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

}  // namespace Strategies
}  // namespace StateGraph

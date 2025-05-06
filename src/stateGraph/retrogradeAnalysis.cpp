#include <cassert>
#include <iostream>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {

void RetrogradeAnalyse(const std::shared_ptr<Vertex> source,
                       const WinState targetQuality,
                       const std::shared_ptr<Edge> edge) {
  assert(!source->Quality.has_value());

  const bool lastUnlabelledEdge = std::all_of(
      source->Edges.begin(), source->Edges.end(),
      [edge](std::shared_ptr<Edge> otherEdge) {
        return edge->Move == otherEdge->Move || otherEdge->Optimal.has_value();
      });

  switch (targetQuality) {
    case WinState::Lose: {
      assert(!source->Quality.has_value());
      source->Quality = WinState::Win;
      source->SetOptimalMove(edge->Move);
      assert(edge->IsOptimal());
      return;
    }

    case WinState::Win: {
      if (lastUnlabelledEdge) {
        if (source->GetOptimalMove().has_value()) {
          // Draw move was already found
          assert(!source->Quality.has_value());
          source->Quality = WinState::Draw;
          edge->Optimal = false;
        } else {
          // No draw move; only losing moves
          assert(!source->Quality.has_value());
          source->Quality = WinState::Lose;
          source->SetOptimalMove(edge->Move);
        }

      } else {
        edge->Optimal = false;
      }
      return;
    }

    case WinState::Draw: {
      // Provisionally set the optimal move to draw
      source->SetOptimalMove(edge->Move);
      assert(edge->IsOptimal());
      if (lastUnlabelledEdge) {
        assert(!source->Quality.has_value());
        source->Quality = WinState::Draw;
      }
      return;
    }

    default: {
      const std::string msg = std::format("Unexpected target quality \"{}\"!",
                                          (int8_t)targetQuality);
      throw std::runtime_error(msg);
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

        // Cannot label an edge with an unlabelled target
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);
        if (!target->Quality.has_value()) continue;

        const bool previouslyLabelled = edge->Optimal.has_value();
        RetrogradeAnalyse(vertex, target->Quality.value(), edge);
        edgeLabelled |= previouslyLabelled != edge->Optimal.has_value();
        assert(edge->Optimal.has_value());

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
      if (vertex->Quality.has_value()) {
        removeVertex(vertexIt);
        continue;
      };

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

          assert((target->Quality.value() != WinState::Lose) &&
                 "Should have been a Win!");
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
      assert((currentVertexRemoved || vertex->GetOptimalMove().has_value()) &&
             "Should have been a Loss!");

      if (!currentVertexRemoved) vertexIt++;
    }
  } while (anyVertexRemoved);

  // Mark unlabelled expanded vertices with only expanded children as draws
  for (const std::shared_ptr<Vertex> draw : unlabelledExpandedVertices) {
    assert(!draw->Quality.has_value() ||
           draw->Quality.value() == WinState::Draw);
    draw->Quality = WinState::Draw;
    assert(draw->GetOptimalMove().has_value());
  }
}

}  // namespace Strategies
}  // namespace StateGraph

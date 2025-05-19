#include <cassert>
#include <iostream>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {

static bool VerifyCorrectness(const Graph& graph) {
  for (const auto [game, vertex] : graph.Vertices) {
    if (!vertex->Quality.has_value()) {
      bool hasUnlabelledChild = false;
      for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
        const std::shared_ptr<const Vertex> target = edge->Target.lock();
        assert(target != nullptr);

        assert(target->Quality != WinState::Lose);

        hasUnlabelledChild |= !target->Quality.has_value();
      }

      assert(vertex->Edges.empty() || hasUnlabelledChild);

      continue;
    }

    if (vertex->Edges.empty()) {
      assert(vertex->Quality == WinState::Lose);
      continue;
    }

    assert(vertex->GetOptimalMove().has_value());
    const Game::Move optimalMove = vertex->GetOptimalMove().value();

    assert(vertex->GetEdge(optimalMove).has_value());
    const std::shared_ptr<const Edge> optimalEdge =
        vertex->GetEdge(optimalMove).value();

    const std::shared_ptr<const Vertex> optimalTarget =
        optimalEdge->Target.lock();
    assert(optimalTarget != nullptr);

    switch (vertex->Quality.value()) {
      case WinState::Win: {
        assert(optimalTarget->Quality == WinState::Lose);

        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          assert(!(edge != optimalEdge && edge->IsOptimal()));
        }

        continue;
      }

      case WinState::Draw: {
        assert(optimalTarget->Quality == WinState::Draw);

        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          assert(!(edge != optimalEdge && edge->IsOptimal()));

          const std::shared_ptr<const Vertex> target = edge->Target.lock();
          assert(target != nullptr);

          assert(target->Quality.has_value());
          assert(target->Quality != WinState::Lose);
        }

        continue;
      }

      case WinState::Lose: {
        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          if (edge != optimalEdge && edge->IsOptimal()) {
            std::cerr << "Error!" << std::endl;
          }
          assert(!(edge != optimalEdge && edge->IsOptimal()));

          const std::shared_ptr<const Vertex> target = edge->Target.lock();
          assert(target != nullptr);

          assert(target->Quality == WinState::Win);
        }

        continue;
      }
    }
  }

  return true;
}

void RetrogradeAnalyse(const std::shared_ptr<Vertex> source,
                       const WinState targetQuality,
                       const std::shared_ptr<Edge> edge) {
  assert(!source->Quality.has_value());
  assert(!edge->Optimal.has_value());
  assert(source->GetEdge(edge->Move).has_value());

  const bool lastUnlabelledEdge = std::all_of(
      source->Edges.begin(), source->Edges.end(),
      [edge](std::shared_ptr<Edge> otherEdge) {
        return edge->Move == otherEdge->Move || otherEdge->Optimal.has_value();
      });
  assert(!(lastUnlabelledEdge && edge->Optimal.has_value()));

  switch (targetQuality) {
    case WinState::Lose: {
      assert(!source->Quality.has_value());
      source->Quality = WinState::Win;
      source->SetOptimalMove(edge->Move);
      return;
    }

    case WinState::Win: {
      if (lastUnlabelledEdge) {
        // Check if there is any draw node we'd rather take
        for (const std::shared_ptr<Edge> otherEdge : source->Edges) {
          const std::shared_ptr<Vertex> target = edge->Target.lock();
          if (target->Quality == WinState::Draw) {
            assert(!source->Quality.has_value());
            source->Quality = WinState::Draw;
            source->SetOptimalMove(otherEdge->Move);
            return;
          }
          assert(target->Quality != WinState::Lose);
        }

        assert(!source->Quality.has_value());
        source->Quality = WinState::Lose;
        source->SetOptimalMove(edge->Move);
        assert(edge->Optimal == true);

      } else {
        edge->Optimal = false;
      }

      return;
    }

    case WinState::Draw: {
      if (lastUnlabelledEdge) {
        assert(!source->Quality.has_value());
        source->Quality = WinState::Draw;
        source->SetOptimalMove(edge->Move);
        assert(edge->Optimal == true);
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

static void RemoveNotFullyExpanded(
    std::unordered_set<std::shared_ptr<Vertex>>& unlabelledExpandedVertices) {
  bool anyVertexRemoved = true;
  bool currentVertexRemoved = false;
  const auto removeVertex =
      [&unlabelledExpandedVertices, &anyVertexRemoved, &currentVertexRemoved](
          std::unordered_set<std::shared_ptr<Vertex>>::iterator& vertex) {
        anyVertexRemoved = true;
        currentVertexRemoved = true;
        vertex = unlabelledExpandedVertices.erase(vertex);
      };

  while (anyVertexRemoved) {
    anyVertexRemoved = false;

    for (auto vertexIt = unlabelledExpandedVertices.begin();
         vertexIt != unlabelledExpandedVertices.end();) {
      const std::shared_ptr<Vertex> vertex = *vertexIt;
      currentVertexRemoved = false;

      if (vertex->Quality.has_value()) {
        removeVertex(vertexIt);
        continue;
      }

      for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);

        if (!target->Quality.has_value()) {
          // If there is an unlabelled successor with no edges, it is unexpanded
          const bool unexpanded = target->Edges.empty() ||
                                  !unlabelledExpandedVertices.contains(target);
          if (unexpanded) {
            removeVertex(vertexIt);
            break;
          }
        }
      }

      if (!currentVertexRemoved) vertexIt++;
    }
  }
}

static void AssignDraws(
    std::unordered_set<std::shared_ptr<Vertex>>& unlabelledExpandedVertices) {
  RemoveNotFullyExpanded(unlabelledExpandedVertices);

  const auto isDrawEdge =
      [&unlabelledExpandedVertices](const std::shared_ptr<Edge> edge) {
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);

        return target->Quality == WinState::Draw ||
               unlabelledExpandedVertices.contains(target);
      };

  for (const std::shared_ptr<Vertex> vertex : unlabelledExpandedVertices) {
    const auto edgeIt =
        std::find_if(vertex->Edges.begin(), vertex->Edges.end(), isDrawEdge);
    if (edgeIt == vertex->Edges.end()) {
      std::cerr << "here";
    }
    assert(edgeIt != vertex->Edges.end());

    const std::shared_ptr<Edge> edge = *edgeIt;
    vertex->SetOptimalMove(edge->Move);
    vertex->Quality = WinState::Draw;
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
        if (edge->Optimal.has_value()) continue;

        // Cannot label an edge with an unlabelled target
        const std::shared_ptr<Vertex> target = edge->Target.lock();
        assert(target != nullptr);
        if (!target->Quality.has_value()) continue;

        RetrogradeAnalyse(vertex, target->Quality.value(), edge);
        edgeLabelled |= edge->Optimal.has_value();

        // The vertex' quality has been determined
        if (vertex->Quality.has_value()) {
          unlabelledExpandedVertices.erase(vertex);
          break;
        }
      }
    }
  } while (edgeLabelled);

  AssignDraws(unlabelledExpandedVertices);

  assert(VerifyCorrectness(graph));
}

}  // namespace Strategies
}  // namespace StateGraph

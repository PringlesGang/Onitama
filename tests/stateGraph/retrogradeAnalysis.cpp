#include "retrogradeAnalysis.h"

#include "../../src/stateGraph/strategies.h"
#include "../../src/util/base64.h"
#include "../assertEqual.h"

namespace Tests {
namespace StateGraph {
namespace RetrogradeAnalysis {

using namespace ::StateGraph;
using namespace ::Game;

static std::optional<std::string> VerifyCorrectness(const Graph& graph) {
  for (const auto [game, vertex] : graph.Vertices) {
    if (!vertex->Quality.has_value()) {
      bool hasUnlabelledChild = false;
      for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
        const std::shared_ptr<const Vertex> target = edge->Target.lock();
        if (target == nullptr)
          return "Target of vertex was deleted weak pointer!";

        if (target->Quality == WinState::Lose)
          return "Unlabelled node has edge to losing node!";

        hasUnlabelledChild |= !target->Quality.has_value();
      }

      if (!vertex->Edges.empty() && !hasUnlabelledChild)
        return "Unlabelled node has all neighbouring nodes labelled!";

      continue;
    }

    if (vertex->Edges.empty()) {
      if (vertex->Quality != WinState::Lose) {
        return "Terminal node was not labelled losing!";
      } else {
        continue;
      }
    }

    if (!vertex->GetOptimalMove().has_value())
      return "Labelled node does not have an optimal move!";

    const Move optimalMove = vertex->GetOptimalMove().value();

    if (!vertex->GetEdge(optimalMove).has_value())
      return "Optimal move not among edges!";

    const std::shared_ptr<const Edge> optimalEdge =
        vertex->GetEdge(optimalMove).value();

    const std::shared_ptr<const Vertex> optimalTarget =
        optimalEdge->Target.lock();
    if (optimalTarget == nullptr)
      return "Optimal target is a deleted weak pointer!";

    switch (vertex->Quality.value()) {
      case WinState::Win: {
        if (optimalTarget->Quality != WinState::Lose)
          return "Winning move did not lead to a losing node!";

        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          if (edge != optimalEdge && edge->IsOptimal()) {
            return "There are multiple optimal winning moves!";
          }
        }

        continue;
      }

      case WinState::Draw: {
        if (optimalTarget->Quality != WinState::Draw)
          return "Optimal move out of a draw node did not lead to another draw "
                 "node!";

        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          if (edge != optimalEdge && edge->IsOptimal())
            return "There are multiple optimal draw moves!";

          const std::shared_ptr<const Vertex> target = edge->Target.lock();
          if (target == nullptr)
            return "Target draw node is a deleted weak pointer!";

          if (!target->Quality.has_value())
            return "Target draw node is unlabelled!";

          if (target->Quality == WinState::Lose)
            return "Target draw node is losing!";
        }

        continue;
      }

      case WinState::Lose: {
        for (const std::shared_ptr<const Edge> edge : vertex->Edges) {
          if (edge != optimalEdge && edge->IsOptimal())
            return "There are multiple optimal losing moves!";

          const std::shared_ptr<const Vertex> target = edge->Target.lock();
          if (target == nullptr)
            return "Target winning node is a deleted weak pointer!";

          if (target->Quality != WinState::Win)
            return "Losing node leads to a non-winning node!";
        }

        continue;
      }
    }
  }

  return std::nullopt;
}

static GameSerialization TemplateSerialization;
static std::unique_ptr<const ::Game::Game> TemplateGame;
static std::shared_ptr<::StateGraph::Vertex> TemplateVertex;
/*
    .....  .....
    .2...  ..1..
    .. 0.  .. ..
    .1...  .....
    .....  ..0..

    .SM    .....
    ..m    ..0..
    S..    .. 2.
    ...    ..1..
    1..    .....

    Card 0 Card 1
    .....  .....
    ..1..  .0.2.
    .0 2.  .. ..
    .....  ..1..
    .....  .....
*/

constexpr Move TemplateMove{
    .PawnId = 2, .UsedCard = Card(CardType::Crab), .OffsetId = 2};

std::shared_ptr<Edge> TemplateEdge;

void Init() {
  TemplateSerialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("GJgowIVdB44").value();
  TemplateGame = std::make_unique<const ::Game::Game>(
      ::Game::Game::FromSerialization(TemplateSerialization));
  TemplateVertex =
      std::make_shared<::StateGraph::Vertex>(TemplateSerialization);
  TemplateEdge =
      std::make_shared<Edge>(TemplateVertex, TemplateVertex, TemplateMove);
  TemplateVertex->Edges.push_back(TemplateEdge);
}

int RetrogradeAnalyseEdge() {
  std::shared_ptr<Edge> otherEdge = std::make_shared<Edge>(*TemplateEdge);
  otherEdge->Move =
      Move{.PawnId = 0, .UsedCard = Card(CardType::Boar), .OffsetId = 0};
  TemplateVertex->Edges.push_back(otherEdge);

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = std::nullopt;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Lose, TemplateEdge);

  if (TemplateVertex->Quality != WinState::Win) {
    std::cerr
        << "Non-fully-expanded node with move to Lose node not marked winning!"
        << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != true) {
    std::cerr << "Move to Lose node not marked optimal!" << std::endl;
    return Fail;
  }

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = std::nullopt;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Draw, TemplateEdge);

  if (TemplateVertex->Quality != std::nullopt) {
    std::cerr << "Non-fully-expanded node with move to Draw node was labelled!"
              << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != std::nullopt) {
    std::cerr << "Move from non-fully-expanded node to Draw node was labelled!"
              << std::endl;
    return Fail;
  }

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = std::nullopt;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Win, TemplateEdge);

  if (TemplateVertex->Quality != std::nullopt) {
    std::cerr << "Non-fully-expanded node with move to Win node was labelled!"
              << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != false) {
    std::cerr << "Move from non-fully-expanded node to Win node was not "
                 "labelled redundant!"
              << std::endl;
    return Fail;
  }

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = false;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Lose, TemplateEdge);

  if (TemplateVertex->Quality != WinState::Win) {
    std::cerr
        << "Fully-expanded node with move to Lose node not marked winning!"
        << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != true) {
    std::cerr
        << "Move from fully-expanded node to Lose node not marked optimal!"
        << std::endl;
    return Fail;
  }

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = false;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Draw, TemplateEdge);

  if (TemplateVertex->Quality != WinState::Draw) {
    std::cerr
        << "Fully-expanded node with move to Draw node was not labelled Draw!"
        << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != true) {
    std::cerr << "Move from fully-expanded node to Draw node was not labelled "
                 "optimal!"
              << std::endl;
    return Fail;
  }

  TemplateVertex->Quality = std::nullopt;
  TemplateEdge->Optimal = std::nullopt;
  otherEdge->Optimal = false;
  Strategies::RetrogradeAnalyse(TemplateVertex, WinState::Win, TemplateEdge);

  if (TemplateVertex->Quality != WinState::Lose) {
    std::cerr
        << "Fully-expanded node with move to Win node was not labelled Lose!"
        << std::endl;
    return Fail;
  }

  if (TemplateEdge->Optimal != true) {
    std::cerr << "Fully-expanded node with only moves to winning nodes was not "
                 "labelled optimal!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int RetrogradeAnalyseGraph() {
  const GameSerialization serialization2x2 =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("QYICQAAB").value();
  const GameSerialization serialization2x3 =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("goIDQAAB").value();
  const GameSerialization serialization2x5 =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("BBIIFQAAB").value();

  const ::Game::Game game2x2 =
      ::Game::Game::FromSerialization(serialization2x2);
  const ::Game::Game game2x3 =
      ::Game::Game::FromSerialization(serialization2x3);
  const ::Game::Game game2x5 =
      ::Game::Game::FromSerialization(serialization2x5);

  Graph graph2x2;
  Strategies::ExploreComponent(graph2x2, game2x2, 0);
  Strategies::RetrogradeAnalyse(graph2x2);

  const std::optional<const std::string> message2x2 =
      VerifyCorrectness(graph2x2);
  if (message2x2.has_value()) {
    std::cerr << std::format("2x2: {}", message2x2.value());
    return Fail;
  }

  const std::optional<WinState> quality2x2 =
      graph2x2.Get(game2x2)->lock()->Quality;
  if (quality2x2 != WinState::Win) {
    const std::string winString =
        quality2x2.has_value() ? to_string(quality2x2.value()) : "Unknown";
    std::cerr << std::format("Expected 2x2 to be winning for Red; got {}!",
                             winString)
              << std::endl;
    return Fail;
  }

  Graph graph2x3;
  Strategies::ExploreComponent(graph2x3, game2x3, 0);
  Strategies::RetrogradeAnalyse(graph2x3);

  const std::optional<const std::string> message2x3 =
      VerifyCorrectness(graph2x3);
  if (message2x2.has_value()) {
    std::cerr << std::format("2x3: {}", message2x3.value());
    return Fail;
  }

  const std::optional<WinState> quality2x3 =
      graph2x3.Get(game2x3)->lock()->Quality;
  if (quality2x3 != WinState::Lose) {
    const std::string winString =
        quality2x3.has_value() ? to_string(quality2x3.value()) : "Unknown";
    std::cerr << std::format("Expected 2x3 to be losing for Red; got {}!",
                             winString)
              << std::endl;
    return Fail;
  }

  Graph graph2x5;
  Strategies::ExploreComponent(graph2x5, game2x5, 0);
  Strategies::RetrogradeAnalyse(graph2x5);

  const std::optional<const std::string> message2x5 =
      VerifyCorrectness(graph2x5);
  if (message2x5.has_value()) {
    std::cerr << std::format("2x5: {}", message2x5.value());
    return Fail;
  }

  const std::optional<WinState> quality2x5 =
      graph2x5.Get(game2x5)->lock()->Quality;
  if (quality2x5 != WinState::Draw) {
    const std::string winString =
        quality2x5.has_value() ? to_string(quality2x5.value()) : "Unknown";
    std::cerr << std::format("Expected 2x5 to be a draw; got {} for Red!",
                             winString)
              << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace RetrogradeAnalysis
}  // namespace StateGraph
}  // namespace Tests

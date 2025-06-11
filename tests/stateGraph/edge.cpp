#include "edge.h"

#include "../../src/stateGraph/stateGraph.h"
#include "../../src/util/base64.h"
#include "../assertEqual.h"

namespace Tests {
namespace StateGraph {
namespace Edge {

using namespace ::Game;

static constexpr std::optional<WinState> TemplateSourceQuality(WinState::Draw);
static GameSerialization TemplateSourceSerialization;
static std::unique_ptr<const ::Game::Game> TemplateSourceGame;
static std::shared_ptr<::StateGraph::Vertex> TemplateSourceVertex;
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

static GameSerialization TemplateTargetSerialization;
static std::unique_ptr<const ::Game::Game> TemplateTargetGame;
static std::shared_ptr<::StateGraph::Vertex> TemplateTargetVertex;
/*
    Card 0 Card 1
    .....  .....
    .2...  ..1..
    .. 0.  .. ..
    .1...  .....
    .....  ..0..

    .mM    .....
    ...    ..1..
    1..    .. ..
    ...    .2.0.
    s..    .....

    .....  .....
    ..1..  ..0..
    .0 2.  .. 2.
    .....  ..1..
    .....  .....
*/

constexpr Move TemplateMove{
    .PawnId = 2, .UsedCard = Card(CardType::Crab), .OffsetId = 2};

void Init() {
  TemplateSourceSerialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("GJgowIVdB44").value();
  TemplateTargetSerialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("GJgOIwVeB41").value();

  TemplateSourceGame = std::make_unique<const ::Game::Game>(
      ::Game::Game::FromSerialization(TemplateSourceSerialization));
  TemplateTargetGame = std::make_unique<const ::Game::Game>(
      ::Game::Game::FromSerialization(TemplateTargetSerialization));

  TemplateSourceVertex = std::make_shared<::StateGraph::Vertex>(
      TemplateSourceSerialization, TemplateSourceQuality);
  TemplateTargetVertex =
      std::make_shared<::StateGraph::Vertex>(*TemplateTargetGame);
}

int Constructor() {
  const std::optional<bool> expectedOptimal = false;
  ::StateGraph::Edge edge(TemplateSourceVertex, TemplateTargetVertex,
                          TemplateMove, expectedOptimal);

  if (edge.Source.lock() != TemplateSourceVertex) {
    std::cerr << "Source vertex differed from expected source on first!"
              << std::endl;
    return Fail;
  }

  if (edge.Target.lock() != TemplateTargetVertex) {
    std::cerr << "Target vertex differed from expected target on first!"
              << std::endl;
    return Fail;
  }

  if (edge.Move != TemplateMove) {
    std::cerr << "Unexpected move on first!" << std::endl;
    return Fail;
  }

  if (edge.Optimal != expectedOptimal) {
    std::cerr << "Unexpected optimality on first!" << std::endl;
    return Fail;
  }

  const Move secondMove{
      .PawnId = 2, .UsedCard = Card(CardType::Frog), .OffsetId = 3};
  edge = ::StateGraph::Edge(TemplateSourceVertex, TemplateSourceVertex,
                            secondMove);

  if (edge.Source.lock() != TemplateSourceVertex) {
    std::cerr << "Source vertex differed from expected source on second!"
              << std::endl;
    return Fail;
  }

  if (edge.Target.lock() != TemplateSourceVertex) {
    std::cerr << "Target vertex differed from expected target on second!"
              << std::endl;
    return Fail;
  }

  if (edge.Move != secondMove) {
    std::cerr << "Unexpected move on second!" << std::endl;
    return Fail;
  }

  if (edge.Optimal != std::nullopt) {
    std::cerr << "Unexpected optimality on second!" << std::endl;
    return Fail;
  }

  return Pass;
}

int IsOptimal() {
  ::StateGraph::Edge edge(TemplateSourceVertex, TemplateTargetVertex,
                          TemplateMove, false);

  if (edge.IsOptimal()) {
    std::cerr << "Redundant edge considered optimal!" << std::endl;
    return Fail;
  }

  edge.Optimal = std::nullopt;

  if (edge.IsOptimal()) {
    std::cerr << "Unknown edge considered optimal!" << std::endl;
    return Fail;
  }

  edge.Optimal = true;

  if (!edge.IsOptimal()) {
    std::cerr << "Optimal edge not considered optimal!" << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace Edge
}  // namespace StateGraph
}  // namespace Tests

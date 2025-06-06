#include "vertex.h"

#include "../../src/stateGraph/stateGraph.h"
#include "../../src/util/base64.h"
#include "../assertEqual.h"

namespace Tests {
namespace StateGraph {
namespace Vertex {

using namespace ::Game;

static constexpr std::optional<WinState> TemplateQuality(WinState::Draw);
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

static GameSerialization TemplateFinishedSerialization;
static std::unique_ptr<const ::Game::Game> TemplateFinishedGame;
static std::shared_ptr<::StateGraph::Vertex> TemplateFinishedVertex;
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

constexpr Move U2FMove{
    .PawnId = 2, .UsedCard = Card(CardType::Crab), .OffsetId = 2};
std::shared_ptr<::StateGraph::Edge> U2FEdge;

constexpr Move U2UMove{
    .PawnId = 1, .UsedCard = Card(CardType::Ox), .OffsetId = 0};
std::shared_ptr<::StateGraph::Edge> U2UEdge;

constexpr Move F2UMove{
    .PawnId = 0, .UsedCard = Card(CardType::Elephant), .OffsetId = 1};
std::shared_ptr<::StateGraph::Edge> F2UEdge;

constexpr Move F2FMove{
    .PawnId = 0, .UsedCard = Card(CardType::Goose), .OffsetId = 1};
std::shared_ptr<::StateGraph::Edge> F2FEdge;

void Init() {
  TemplateSerialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("GJgowIVdB44").value();
  TemplateFinishedSerialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>("GJgOIwVeB41").value();

  TemplateGame = std::make_unique<const ::Game::Game>(
      ::Game::Game::FromSerialization(TemplateSerialization));
  TemplateFinishedGame = std::make_unique<const ::Game::Game>(
      ::Game::Game::FromSerialization(TemplateFinishedSerialization));

  TemplateVertex = std::make_shared<::StateGraph::Vertex>(TemplateSerialization,
                                                          TemplateQuality);
  TemplateFinishedVertex =
      std::make_shared<::StateGraph::Vertex>(*TemplateFinishedGame);

  U2FEdge = std::make_shared<::StateGraph::Edge>(
      TemplateVertex, TemplateFinishedVertex, U2FMove, true);
  U2UEdge = std::make_shared<::StateGraph::Edge>(TemplateVertex, TemplateVertex,
                                                 U2UMove, false);
  F2UEdge = std::make_shared<::StateGraph::Edge>(
      TemplateFinishedVertex, TemplateVertex, F2UMove, std::nullopt);
  F2FEdge = std::make_shared<::StateGraph::Edge>(
      TemplateFinishedVertex, TemplateFinishedVertex, F2FMove, false);

  TemplateVertex->Edges = {U2FEdge, U2UEdge};
  TemplateFinishedVertex->Edges = {F2UEdge, F2FEdge};
}

int GameConstructor() {
  const ::StateGraph::Vertex unfinishedVertex(*TemplateGame);

  if (!unfinishedVertex.Edges.empty()) {
    std::cerr << "Unfinished game automatically added edges!" << std::endl;
    return Fail;
  }

  if (unfinishedVertex.Serialization != TemplateSerialization) {
    std::cerr
        << std::format(
               "Expected serialization \"{}\" for unfinished game; got \"{}\"!",
               Base64::Encode(TemplateSerialization),
               Base64::Encode(unfinishedVertex.Serialization))
        << std::endl;
    return Fail;
  }

  if (unfinishedVertex.Quality != std::nullopt) {
    std::cerr << std::format("Expected no quality for unfinished game; got {}!",
                             to_string(unfinishedVertex.Quality.value()))
              << std::endl;
  }

  const ::StateGraph::Vertex finishedVertex(*TemplateFinishedGame);

  if (!finishedVertex.Edges.empty()) {
    std::cerr << "Finished game automatically added edges!" << std::endl;
    return Fail;
  }

  if (finishedVertex.Serialization != TemplateFinishedSerialization) {
    std::cerr
        << std::format(
               "Expected serialization \"{}\" for finished game; got \"{}\"!",
               Base64::Encode(TemplateFinishedSerialization),
               Base64::Encode(finishedVertex.Serialization))
        << std::endl;
    return Fail;
  }

  if (finishedVertex.Quality != WinState::Lose) {
    std::cerr << std::format(
                     "Expected losing quality for finished game; got {}!",
                     finishedVertex.Quality.has_value()
                         ? to_string(*finishedVertex.Quality)
                         : "unknown")
              << std::endl;
    return Fail;
  }

  return Pass;
}

int SerializationConstructor() {
  const ::StateGraph::Vertex unknownVertex(TemplateSerialization);

  if (!unknownVertex.Edges.empty()) {
    std::cerr << "Automatically added edges to unknown vertex!" << std::endl;
    return Fail;
  }

  if (unknownVertex.Serialization != TemplateSerialization) {
    std::cerr
        << std::format(
               "Expected serialization \"{}\" for unknown vertex; got \"{}\"!",
               Base64::Encode(TemplateSerialization),
               Base64::Encode(unknownVertex.Serialization))
        << std::endl;
    return Fail;
  }

  if (unknownVertex.Quality != std::nullopt) {
    std::cerr << std::format(
                     "Expected unknown quality for unknown game; got {}!",
                     to_string(unknownVertex.Quality.value()))
              << std::endl;
    return Fail;
  }

  const ::StateGraph::Vertex winningFinishedVertex(
      TemplateFinishedSerialization, WinState::Win);

  if (!winningFinishedVertex.Edges.empty()) {
    std::cerr << "Automatically added edges to winning finished vertex!"
              << std::endl;
    return Fail;
  }

  if (winningFinishedVertex.Serialization != TemplateFinishedSerialization) {
    std::cerr << std::format(
                     "Expected serialization \"{}\" for winning finished "
                     "vertex; got \"{}\"!",
                     Base64::Encode(TemplateFinishedSerialization),
                     Base64::Encode(winningFinishedVertex.Serialization))
              << std::endl;
    return Fail;
  }

  if (winningFinishedVertex.Quality != WinState::Win) {
    std::cerr
        << std::format(
               "Expected winning quality for winning finished game; got {}!",
               winningFinishedVertex.Quality.has_value()
                   ? to_string(*winningFinishedVertex.Quality)
                   : "unknown")
        << std::endl;
    return Fail;
  }

  const ::StateGraph::Vertex drawVertex(TemplateSerialization, WinState::Draw);

  if (!drawVertex.Edges.empty()) {
    std::cerr << "Automatically added edges to draw vertex!" << std::endl;
    return Fail;
  }

  if (drawVertex.Serialization != TemplateSerialization) {
    std::cerr
        << std::format(
               "Expected serialization \"{}\" for draw vertex; got \"{}\"!",
               Base64::Encode(TemplateSerialization),
               Base64::Encode(drawVertex.Serialization))
        << std::endl;
    return Fail;
  }

  if (drawVertex.Quality != WinState::Draw) {
    std::cerr << std::format("Expected draw quality for unknown game; got {}!",
                             drawVertex.Quality.has_value()
                                 ? to_string(*drawVertex.Quality)
                                 : "unknown")
              << std::endl;
    return Fail;
  }

  return Pass;
}

int EqualityOperator() {
  if (!(*TemplateVertex == *TemplateVertex)) {
    std::cerr << "Two equal vertices were deemed not equal!" << std::endl;
    return Fail;
  }

  if (*TemplateVertex == *TemplateFinishedVertex) {
    std::cerr << "Two inequal vertices were deemed equal!" << std::endl;
    return Fail;
  }

  ::StateGraph::Vertex moveFewerVertex = *TemplateVertex;
  moveFewerVertex.Edges.pop_back();
  if (!(*TemplateVertex == moveFewerVertex)) {
    std::cerr << "Equality operator took outgoing edges into account!"
              << std::endl;
    return Fail;
  }

  ::StateGraph::Vertex losingVertex = *TemplateVertex;
  losingVertex.Quality = WinState::Lose;
  if (!(*TemplateVertex == losingVertex)) {
    std::cerr << "Equality operator took quality into account!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GameEqualityOperator() {
  if (!(*TemplateVertex == *TemplateGame)) {
    std::cerr
        << "Unfinished vertex was not deemed equal to its unfinished game!"
        << std::endl;
    return Fail;
  }

  if (*TemplateVertex == *TemplateFinishedGame) {
    std::cerr << "Unfinished vertex was deemed equal to finished game!"
              << std::endl;
    return Fail;
  }

  if (!(*TemplateFinishedVertex == *TemplateFinishedGame)) {
    std::cerr << "Finished vertex was not deemed equal to its finished game!"
              << std::endl;
    return Fail;
  }

  if (*TemplateFinishedVertex == *TemplateGame) {
    std::cerr << "Finished vertex was deemed equal to unfinished game!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int SetOptimalMove() {
  const Move optimalUnfinishedMove = U2UMove;

  if (TemplateVertex->GetEdge(optimalUnfinishedMove).value()->IsOptimal()) {
    std::cerr << "Initial optimal move for unfinished move improperly set!"
              << std::endl;
    return Fail;
  }

  TemplateVertex->SetOptimalMove(optimalUnfinishedMove);

  if (!TemplateVertex->GetEdge(optimalUnfinishedMove).value()->IsOptimal()) {
    std::cerr << "Unfinished move was not properly set!" << std::endl;
    return Fail;
  }

  const Move optimalFinishedMove = F2FMove;

  if (TemplateFinishedVertex->GetEdge(optimalFinishedMove)
          .value()
          ->IsOptimal()) {
    std::cerr << "Initial optimal move for finished move improperly set!"
              << std::endl;
    return Fail;
  }

  TemplateFinishedVertex->SetOptimalMove(optimalFinishedMove);

  if (!TemplateFinishedVertex->GetEdge(optimalFinishedMove)
           .value()
           ->IsOptimal()) {
    std::cerr << "Finished move was not properly set!" << std::endl;
    return Fail;
  }

  return Pass;
}

int GetOptimalMove() {
  if (!TemplateVertex->GetEdge(TemplateVertex->GetOptimalMove().value())
           .value()
           ->IsOptimal()) {
    std::cerr << "Vertex with optimal move did not return an optimal move!"
              << std::endl;
    return Fail;
  }

  if (TemplateFinishedVertex->GetOptimalMove() != std::nullopt) {
    std::cerr << "Vertex without optimal move returned optimal move!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetEdgeMove() {
  if (TemplateVertex->GetEdge(U2UMove) != U2UEdge) {
    std::cerr << "Get edge did not return the requested edge!" << std::endl;
    return Fail;
  }

  if (TemplateVertex->GetEdge(F2UMove) != std::nullopt) {
    std::cerr << "Get edge did not return nullopt on non-outgoing edge!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

int GetEdgeSerialization() {
  if (TemplateVertex->GetEdge(TemplateFinishedSerialization) != U2FEdge) {
    std::cerr << "Get edge did not return requested edge!" << std::endl;
    return Fail;
  }

  if (TemplateVertex->GetEdge(GameSerialization()) != std::nullopt) {
    std::cerr << "Get edge did not return nullopt on non-outgoing edge!"
              << std::endl;
    return Fail;
  }

  return Pass;
}

}  // namespace Vertex
}  // namespace StateGraph
}  // namespace Tests

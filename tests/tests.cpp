#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "./game/board.h"
#include "./game/game.h"
#include "./stateGraph/edge.h"
#include "./stateGraph/retrogradeAnalysis.h"
#include "./stateGraph/saveSystem.h"
#include "./stateGraph/vertex.h"

namespace Tests {

struct CaseInsensitiveHash {
  size_t operator()(std::string string) const noexcept {
    std::transform(string.begin(), string.end(), string.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return std::hash<std::string>()(string);
  }
};

struct CaseInsensitiveEqual {
  bool operator()(std::string first, std::string second) const noexcept {
    std::transform(first.begin(), first.end(), first.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(second.begin(), second.end(), second.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return first == second;
  }
};

const static std::unordered_map<std::string, std::function<int()>,
                                CaseInsensitiveHash, CaseInsensitiveEqual>
    Tests = {
        {"Game::Game initial state constructor",
         Game::Game::InitialStateConstructor},
        {"Game::Game from board constructor", Game::Game::BoardConstructor},
        {"Game::Game copy constructor", Game::Game::CopyConstructor},
        {"Game::Game move constructor", Game::Game::MoveConstructor},
        {"Game::Game::WithRandomCards", Game::Game::WithRandomCards},
        {"Game::Game::FromSerialization", Game::Game::FromSerialization},
        {"Game::Game::ParseSerialization pass", Game::Game::ParseSerialization},
        {"Game::Game::ParseSerialization fail",
         Game::Game::ParseIncorrectSerialization},
        {"Game::Game::operator==", Game::Game::Equality},
        {"Game::Game::operator!=", Game::Game::Inequality},
        {"Game::Game::GetHand color", Game::Game::GetHand},
        {"Game::Game::GetHand current", Game::Game::GetCurrentHand},
        {"Game::Game::GetSetAsideCard", Game::Game::GetSetAsideCard},
        {"Game::Game::GetCurrentPlayer", Game::Game::GetCurrentPlayer},
        {"Game::Game::GetDimensions", Game::Game::GetDimensions},
        {"Game::Game::GetPawnCoordinates color",
         Game::Game::GetPawnCoordinates},
        {"Game::Game::GetPawnCoordinates current",
         Game::Game::GetCurrentPawnCoordinates},
        {"Game::Game::MasterCaptured color", Game::Game::MasterCaptured},
        {"Game::Game::MasterCaptured current",
         Game::Game::CurrentMasterCaptured},
        {"Game::Game::GetPawnCount color", Game::Game::GetPawnCount},
        {"Game::Game::GetPawnCount current", Game::Game::GetCurrentPawnCount},
        {"Game::Game::GetValidMoves", Game::Game::GetValidMoves},
        {"Game::Game::HasValidMoves", Game::Game::HasValidMoves},
        {"Game::Game::IsValidMove", Game::Game::IsValidMove},
        {"Game::Game::IsFinished", Game::Game::IsFinished},
        {"Game::Game::DoMove", Game::Game::DoMove},
        {"Game::Game::Serialize", Game::Game::Serialize},

        {"Game::Board::InitialStateConstructorEvenWidth",
         Game::Board::InitialStateConstructorEvenWidth},
        {"Game::Board::InitialStateConstructorOddWidth",
         Game::Board::InitialStateConstructorOddWidth},
        {"Game::Board::GridConstructor", Game::Board::GridConstructor},
        {"Game::Board::CopyConstructor", Game::Board::CopyConstructor},
        {"Game::Board::MoveConstructor", Game::Board::MoveConstructor},
        {"Game::Board::EqualityOperator", Game::Board::EqualityOperator},
        {"Game::Board::Reset", Game::Board::Reset},
        {"Game::Board::DoMove", Game::Board::DoMove},
        {"Game::Board::GetGrid", Game::Board::GetGrid},
        {"Game::Board::GetTile", Game::Board::GetTile},
        {"Game::Board::GetRow", Game::Board::GetRow},
        {"Game::Board::GetPawnCoordinates", Game::Board::GetPawnCoordinates},
        {"Game::Board::GetDimensions", Game::Board::GetDimensions},
        {"Game::Board::GetTemple", Game::Board::GetTemple},
        {"Game::Board::MasterCaptured", Game::Board::MasterCaptured},
        {"Game::Board::OnBoard", Game::Board::OnBoard},
        {"Game::Board::IsFinished", Game::Board::IsFinished},

        {"StateGraph::Vertex::GameConstructor",
         StateGraph::Vertex::GameConstructor},
        {"StateGraph::Vertex::SerializationConstructor",
         StateGraph::Vertex::SerializationConstructor},
        {"StateGraph::Vertex::EqualityOperator",
         StateGraph::Vertex::EqualityOperator},
        {"StateGraph::Vertex::GameEqualityOperator",
         StateGraph::Vertex::GameEqualityOperator},
        {"StateGraph::Vertex::SetOptimalMove",
         StateGraph::Vertex::SetOptimalMove},
        {"StateGraph::Vertex::GetOptimalMove",
         StateGraph::Vertex::GetOptimalMove},
        {"StateGraph::Vertex::GetEdgeMove", StateGraph::Vertex::GetEdgeMove},
        {"StateGraph::Vertex::GetEdgeSerialization",
         StateGraph::Vertex::GetEdgeSerialization},

        {"StateGraph::Edge::Constructor", StateGraph::Edge::Constructor},
        {"StateGraph::Edge::IsOptimal", StateGraph::Edge::IsOptimal},

        {"StateGraph::Graph::Save", StateGraph::Save},
        {"StateGraph::Graph::Load", StateGraph::Load},

        {"StateGraph::RetrogradeAnalysis::RetrogradeAnalyseEdge",
         StateGraph::RetrogradeAnalysis::RetrogradeAnalyseEdge},
        {"StateGraph::RetrogradeAnalysis::RetrogradeAnalyseGraph",
         StateGraph::RetrogradeAnalysis::RetrogradeAnalyseGraph},
};

int RunAll() {
  int exitCode = 0;
  for (const auto& [id, test] : Tests) {
    std::cout << std::format("Performing test \"{}\"... \t", id);

    int result = 0;
    try {
      result = test();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }

    std::cout << (result ? "Failed!" : "Passed!") << std::endl;
    exitCode |= result;
  }

  return exitCode;
}

int Run(const std::string& id) {
  if (!Tests.contains(id)) {
    std::cerr << std::format("Unknown test \"{}\"!", id) << std::endl;
    return 1;
  }

  try {
    return Tests.at(id)();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

void Init() {
  Game::Game::Init();
  Game::Board::Init();
  StateGraph::Vertex::Init();
  StateGraph::Edge::Init();
  StateGraph::RetrogradeAnalysis::Init();
}

}  // namespace Tests

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << "No test provided!" << std::endl;
    return 1;
  }

  Tests::Init();

  std::string command = "";
  for (int arg = 1; arg < argc; arg++) {
    if (arg != 1) command += " ";
    command += std::string(argv[arg]);
  }
  std::transform(command.begin(), command.end(), command.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (command == "--all") return Tests::RunAll();

  return Tests::Run(command);
}

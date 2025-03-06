#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../cli/game.h"
#include "../util/base64.h"
#include "../util/stateGraph.h"

namespace Experiments {
namespace StateGraph {

void Execute(Game::Game game) {
  std::cout << std::format("Generating state graph for {}:\n",
                           Base64::Encode(game.Serialize()))
            << game << std::endl;

  std::shared_ptr<::StateGraph::Graph> graph =
      std::make_shared<::StateGraph::Graph>();

  graph->Add(Game::Game(game));

  const std::shared_ptr<const ::StateGraph::Vertex> vertex =
      graph->Get(game)->lock();

  switch (vertex->Quality) {
    case WinState::Lost:
      std::cout << "Lost" << std::endl;
      return;
    case WinState::Unknown:
      std::cout << "Tie" << std::endl;
      return;
    case WinState::Won:
      std::cout << "Won" << std::endl;
      return;
  }
}

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  std::string argument;
  command >> argument;

  if (argument.empty()) {
    std::cout << "No game provided!" << std::endl;
    return std::nullopt;
  }

  if (argument == "--state") {
    return ParseSerialization(command);
  } else if (argument == "--game") {
    return ParseGame(command);
  } else {
    std::cout << std::format("Unknown argument \"{}\"!", argument) << std::endl;
    return std::nullopt;
  }
}

std::optional<Cli::Thunk> ParseGame(std::istringstream& command) {
  Cli::GameArgs gameArgs;

  std::string argument;
  command >> argument;
  while (!argument.empty()) {
    if (argument == "--duplicate-cards" || argument == "-d") {
      gameArgs.RepeatCards = true;

    } else if (argument == "--cards" || argument == "-c") {
      if (!Cli::GameCommand::ParseCards(command, gameArgs)) return std::nullopt;

    } else if (argument == "--size" || argument == "-s") {
      if (!Cli::GameCommand::ParseDimensions(command, gameArgs))
        return std::nullopt;

    } else {
      std::cout << std::format("Unknown argument \"{}\"!", argument)
                << std::endl;
      return std::nullopt;
    }

    argument.clear();
    command >> argument;
  }

  return [gameArgs] { Execute(std::move(gameArgs.ToGame())); };
}

std::optional<Cli::Thunk> ParseSerialization(std::istringstream& command) {
  std::string serializationString;
  command >> serializationString;

  if (!Cli::Command::Terminate(command)) return std::nullopt;

  Game::GameSerialization serialization;
  try {
    serialization = Base64::Decode<Game::GAME_SERIALIZATION_SIZE>(
        std::move(serializationString));
  } catch (std::runtime_error err) {
    std::cout << std::format("Failed to parse base64 string \"{}\"",
                             serializationString)
              << std::endl;
    return std::nullopt;
  }

  try {
    const Game::Game game = Game::Game::FromSerialization(serialization);
    return [game] { Execute(game); };
  } catch (std::runtime_error err) {
    std::cout << err.what() << std::endl;
    return std::nullopt;
  }
}

}  // namespace StateGraph
}  // namespace Experiments

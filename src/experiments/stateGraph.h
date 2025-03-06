#pragma once

#include "../cli/command.h"
#include "../game/game.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view Command =
    "stategraph --state game_serialization\n"

    "stategraph --game (--duplicate-cards) (--cards set_aside r1 r2 b1 b2) "
    "(--size width height)";

constexpr inline std::string_view Description =
    "Depending on the arguments, "
    "will construct the winning state graph for either:\n"
    "- the provided game state;\n"
    "- or the starting configuration with the supplied arguments.";

void Execute(Game::Game game);

std::optional<Cli::Thunk> Parse(std::istringstream& command);
std::optional<Cli::Thunk> ParseGame(std::istringstream& command);
std::optional<Cli::Thunk> ParseSerialization(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments

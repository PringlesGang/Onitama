#pragma once

#include "../../cli/command.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view Command =
    "stategraph strategy state game_serialization (--export filepath) "
    "(--import filepath) (--intermediate path seconds-interval) (--data)\n"

    "stategraph strategy game (--duplicate-cards) "
    "(--cards set_aside r1 r2 b1 b2) (--size width height) "
    "(--export nodes-path edges-path) (--import nodes-path edges-path) "
    "(--intermediate path seconds-interval) (--data)\n"

    "stategraph strategy load intermediate_path "
    "(--export nodes-path edges-path) (--intermediate path seconds-interval) "
    "(--data)";

constexpr inline std::string_view Description =
    "Depending on the arguments, "
    "will construct the state graph for either:\n"
    "- the provided game state;\n"
    "- the starting configuration with the supplied arguments;\n"
    "- the loaded intermediate state graph.\n\n"

    "Will export the graph to the provided filepath.\n"
    "Will import the graph from the provided filepath.\n"
    "Will repeatedly save intermediate results to the provided filepath after "
    "the supplied duration.\n\n"

    "--strategy will construct the state graph through the given strategy.\n\n"

    "--data will print the output as csv data.";

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments

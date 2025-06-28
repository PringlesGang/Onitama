#pragma once

#include "../../cli/command.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view HelpEntry =
    "experiment stategraph  Generates and analyses a specific state graph\n"
    "                       from a specific game state.\n";

constexpr inline std::string_view Help =
    "experiment stategraph <construction_strategy> game [game_options]\n"
    "   [general_options]\n"
    "experiment stategraph <construction_strategy> state <state_id>\n"
    "   [general_options]\n"
    "\n"
    "Constructs a state graph for the supplied game or the given game\n"
    "state.\n"
    "\n"
    "Game options are the same as for the \"game\" command.\n"
    "\n"
    "General options:\n"
    "-e, --export <nodes-path> <edges-path>\n"
    "                        Export the constructed state graph after it\n"
    "                        is done.\n"
    "-i, --import <nodes-path> <edges-path>\n"
    "                        Import (part of) a state graph.\n"
    "--images <images-path>  Export small images of each game state to\n"
    "                        the provided directory.\n"
    "--intermediate <file-path> <save-interval>\n"
    "                        Save an intermediate state graph every \n"
    "                        <save-interval> seconds.\n"
    "--load <file-path>      Load an intermediate state graph.\n"
    "--disable-symmetries    Count each game state as distinct, and do not\n"
    "                        apply symmetries to cut down on the amount of\n"
    "                        game states that need to be analysed.\n"
    "--data                  Print the output data in csv format.\n"
    "\n"
    "Construction strategies:\n"
    "component <max_depth>   Map out all reachable game states from the\n"
    "                        initial state. <max_depth> denotes the maximum\n"
    "                        recursion depth.\n"
    "retrograde-analysis, retrograde <max_depth>\n"
    "                        Construct a state graph using the `component`\n"
    "                        strategy, and analyse it using retrograde\n"
    "                        analysis.\n"
    "forward-retrograde-analysis, forward-retrograde, forward\n"
    "                        Construct a state graph and simultaneously\n"
    "                        analyse it using forward-looking retrograde\n"
    "                        analysis. Exits once it's found an ultra-weak\n"
    "                        solution.\n"
    "dispersed-frontier, dispersed <depth> <max_thread_count>\n"
    "                        Constructs the state graph in parallel,\n"
    "                        exploring up to a depth of <depth> in each\n"
    "                        separate thread. At most <max_thread_count>\n"
    "                        are active at one time.\n";

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments

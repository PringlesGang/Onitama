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
    "Constructs a state graph for the supplied game or the given game "
    "state.\n"
    "\n"
    "Game options are the same as for the \"game\" command.\n"
    "\n"
    "General options:\n"
    "-e, --export <nodes-path> <edges-path>\n"
    "                        Export the constructed state graph after it "
    "is\n"
    "                        done.\n"
    "-i, --import <nodes-path> <edges-path>\n"
    "                        Import (part of) a state graph.\n"
    "--images <images-path>  Export small images of each game state to "
    "the\n"
    "                        provided directory.\n"
    "--intermediate <file-path> <save-interval>\n"
    "                        Save an intermediate state graph every \n"
    "                        <save-interval> seconds.\n"
    "--load <file-path>      Load an intermediate state graph.\n"
    "--disable-symmetries    Count each game state as distinct, and do not "
    "\n"
    "                        apply symmetries to cut down on the amount of "
    "\n"
    "                        game states that need to be analysed.\n"
    "--data                  Print the output data in csv format.\n";

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments

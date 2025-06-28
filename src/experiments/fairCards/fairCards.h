#pragma once

#include "../../cli/command.h"
#include "../../cli/strategies.h"

namespace Experiments {
namespace FairCards {

constexpr inline std::string_view Name = "faircards";

constexpr inline std::string_view HelpEntry =
    "experiment faircards   Tests whether all card combinations are fair.\n";

constexpr inline std::string_view Help =
    "experiment faircards <repeat-count> <strategy>\n"
    "\n"
    "Tests whether all card combinations are fair.\n"
    "<repeat-count> specifies how often each combination needs to be\n"
    "repeated.\n";

void Execute(const size_t repeatCount, const Cli::StrategyFactory strategy);

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace FairCards
}  // namespace Experiments

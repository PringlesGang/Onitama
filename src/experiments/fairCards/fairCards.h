#pragma once

#include "../../cli/command.h"
#include "../../cli/strategies.h"

namespace Experiments {
namespace FairCards {

constexpr inline std::string_view Name = "faircards";

constexpr inline std::string_view Command = "faircards repeat-count strategy";

constexpr inline std::string_view Description =
    "Tests whether all card combinations are fair.\n"
    "repeat-count specified how often each combination needs to be repeated.";

void Execute(const size_t repeatCount, const Cli::StrategyFactory strategy);

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace FairCards
}  // namespace Experiments

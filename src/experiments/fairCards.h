#pragma once

#include "../cli/command.h"

namespace Experiments {
namespace FairCards {

constexpr inline std::string_view Name = "faircards";

constexpr inline std::string_view Command = "faircards repeat-count";

constexpr inline std::string_view Description =
    "Tests whether all card combinations are fair.\n"
    "repeat-count specified how often each combination needs to be repeated.";

void Execute(size_t repeatCount);

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace FairCards
}  // namespace Experiments

#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../../cli/game.h"
#include "../../stateGraph/stateGraph.h"
#include "../../stateGraph/strategies.h"
#include "../../util/base64.h"
#include "../../util/parse.h"
#include "args.h"

namespace Experiments {
namespace StateGraph {

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  const std::optional<std::shared_ptr<StateGraphArgs>> args =
      StateGraphArgs::Parse(command);

  if (!args.has_value()) return std::nullopt;
  if (!args.value()->IsValid()) return std::nullopt;

  return [args] { args.value()->Execute(); };
}

}  // namespace StateGraph
}  // namespace Experiments

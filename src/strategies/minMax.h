#include <functional>

#include "strategy.h"

namespace Strategy {

#pragma warning(suppress : 4091)
static enum class WinState {
  Lost = -1,
  Unknown = 0,
  Won = 1,
};

static WinState operator-(const WinState original);
static bool operator<(const WinState lhs, const WinState rhs);

class MinMax : public Strategy {
 public:
  MinMax(const std::optional<const size_t> maxDepth);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<MinMax>()>> Parse(
      std::istringstream& command);

  static std::string GetName();
  static std::string GetCommand();
  static std::string GetDescription();

 private:
  const std::optional<const size_t> MaxDepth;

  WinState PlayRecursive(Game::Game game, const size_t depth) const;
};

}  // namespace Strategy

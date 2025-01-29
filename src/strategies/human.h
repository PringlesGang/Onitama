#include <optional>

#include "strategy.h"

namespace Strategy {

class Human : Strategy {
 public:
  Game::Move GetMove(const Game::Game& game) override;

 private:
  static std::optional<Game::Move> ParseMove(const std::string& string,
                                             const Game::Game& game);
};

}  // namespace Strategy

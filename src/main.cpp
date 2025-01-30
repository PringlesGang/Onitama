#include <utility>

#include "gameMaster.h"
#include "strategies/human.h"
#include "strategies/random.h"

int main(int argc, char* argv[]) {
  GameMaster game = GameMaster(std::make_unique<Strategy::Random>(),
                               std::make_unique<Strategy::Random>());

  do {
    game.Render();
    game.Update();
  } while (!game.IsFinished());
  game.Render();

  return 0;
}
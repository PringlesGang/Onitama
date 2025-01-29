#include <utility>

#include "gameMaster.h"
#include "strategies/human.h"

int main(int argc, char* argv[]) {
  GameMaster game = GameMaster(std::make_unique<Strategy::Human>(),
                               std::make_unique<Strategy::Human>());

  do {
    game.Render();
    game.Update();
  } while (!game.IsFinished());

  return 0;
}
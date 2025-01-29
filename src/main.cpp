#include <iostream>

#include "game./game.h"

int main(int argc, char* argv[]) {
  Game game = Game::WithRandomCards();

  std::cout << game;

  return 0;
}
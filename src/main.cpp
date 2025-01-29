#include <iostream>

#include "game./game.h"

int main(int argc, char* argv[]) {
  Game::Game game = Game::Game::WithRandomCards();

  std::cout << game;

  return 0;
}
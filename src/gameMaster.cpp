#include "gameMaster.h"

#include <format>
#include <iostream>
#include <stdexcept>

GameMaster::GameMaster(std::unique_ptr<Strategy::Strategy> redPlayer,
                       std::unique_ptr<Strategy::Strategy> bluePlayer,
                       bool repeatCards)
    : RedPlayer(std::move(redPlayer)),
      BluePlayer(std::move(bluePlayer)),
      GameInstance(Game::Game::WithRandomCards(repeatCards)) {}

GameMaster::GameMaster(std::unique_ptr<Strategy::Strategy> redPlayer,
                       std::unique_ptr<Strategy::Strategy> bluePlayer,
                       std::array<Game::Card, CARD_COUNT> cards)
    : RedPlayer(std::move(redPlayer)),
      BluePlayer(std::move(bluePlayer)),
      GameInstance(Game::Game(cards)) {}

void GameMaster::Render() const {
  std::cout << std::format("Round {}:", round) << std::endl << GameInstance;
}

void GameMaster::Update() {
  if (IsFinished()) return;

  Game::Move move;
  switch (GameInstance.GetCurrentPlayer()) {
    case Color::Red:
      move = RedPlayer->GetMove(GameInstance);
      break;
    case Color::Blue:
      move = BluePlayer->GetMove(GameInstance);
      break;

    default:
      const size_t colorNum = (size_t)GameInstance.GetCurrentPlayer();
      throw std::runtime_error(std::format("Invalid color {}", colorNum));
  }

  round += GameInstance.DoMove(move);
}

std::optional<Color> GameMaster::IsFinished() const {
  return GameInstance.IsFinished();
}

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

void GameMaster::PrintData() const {
  if (MoveHistory.empty()) {
    for (Game::Card card : GameInstance.GetCards()) {
      std::cout << std::format("{},", card.GetName());
    }
    std::cout << std::endl;
  } else {
    Game::Move move = MoveHistory.top();
    std::cout << std::format("{},{},{},", move.PawnId, move.UsedCard.GetName(),
                             move.OffsetId)
              << std::endl;
  }
}

void GameMaster::PrintBoard() const {
  std::cout << std::format("Round {}:", Round) << std::endl << GameInstance;
}

void GameMaster::Render() const {
  switch (GameMasterPrintType) {
    case PrintType::Board:
      PrintBoard();
      break;

    case PrintType::Data:
      PrintData();
      break;

    case PrintType::Wins: {
      const std::optional<Color> winner = IsFinished();
      if (winner) {
        std::cout << "Winner: " << winner.value() << std::endl;
      }

      break;
    }

    case PrintType::None:
    default:
      break;
  }
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

  if (GameInstance.DoMove(move)) {
    Round++;
    MoveHistory.push(move);
  }
}

std::optional<Color> GameMaster::IsFinished() const {
  return GameInstance.IsFinished();
}

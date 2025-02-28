#include "gameMaster.h"

#include <format>
#include <iostream>
#include <stdexcept>

GameMaster::GameMaster(const size_t width, const size_t height,
                       std::unique_ptr<Strategy::Strategy> redPlayer,
                       std::unique_ptr<Strategy::Strategy> bluePlayer,
                       bool repeatCards)
    : RedPlayer(std::move(redPlayer)),
      BluePlayer(std::move(bluePlayer)),
      GameInstance(Game::Game::WithRandomCards(width, height, repeatCards)) {}

GameMaster::GameMaster(const size_t width, const size_t height,
                       std::unique_ptr<Strategy::Strategy> redPlayer,
                       std::unique_ptr<Strategy::Strategy> bluePlayer,
                       std::array<Game::Card, CARD_COUNT> cards)
    : RedPlayer(std::move(redPlayer)),
      BluePlayer(std::move(bluePlayer)),
      GameInstance(Game::Game(width, height, cards)) {}

void GameMaster::PrintData(std::ostream& stream) const {
  if (MoveHistory.empty()) {
    for (Game::Card card : GameInstance.GetCards()) {
      stream << std::format("{},", card.GetName());
    }
    stream << std::endl;
  } else {
    Game::Move move = MoveHistory.top();
    stream << std::format("{},{},{},", move.PawnId, move.UsedCard.GetName(),
                          move.OffsetId)
           << std::endl;
  }
}

void GameMaster::PrintBoard(std::ostream& stream) const {
  stream << std::format("Round {}:", Round) << std::endl << GameInstance;
}

void GameMaster::Render(std::ostream& stream) const {
  switch (GameMasterPrintType) {
    case PrintType::Board:
      PrintBoard(stream);
      break;

    case PrintType::Data:
      PrintData(stream);
      break;

    case PrintType::Wins: {
      const std::optional<Color> winner = IsFinished();
      if (winner) {
        stream << "Winner: " << winner.value() << std::endl;
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

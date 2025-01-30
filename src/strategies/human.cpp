#include "human.h"

#include <format>
#include <iostream>
#include <string>

#include "../constants.h"

namespace Strategy {

Game::Move Human::GetMove(const Game::Game& game) {
  std::optional<Game::Move> move;

  if (game.GetValidMoves().size() > 0) {
    do {
      std::cout << std::endl
                << "Please enter your move (`{pawn ID} {card number} {offset "
                   "number}`):"
                << std::endl;

      std::string input;
      getline(std::cin, input);

      move = ParseMove(input, game);
    } while (!move);
  } else {
    do {
      std::cout << std::endl
                << "No valid moves. Please choose a card to discard:"
                << std::endl;

      std::string input;
      getline(std::cin, input);

      move = ParseCard(input, game);
    } while (!move);
  }

  return *move;
}

std::optional<Game::Move> Human::ParseMove(const std::string& string,
                                           const Game::Game& game) {
  char pawnId;
  unsigned int cardNum;
  unsigned int offsetNum;

  const size_t found =
      std::sscanf(string.c_str(), "%c %u %u", &pawnId, &cardNum, &offsetNum);

  if (found != 3) {
    std::cout << "Failed to parse move!" << std::endl;
    return std::nullopt;
  }

  const bool master = pawnId == 'm' || pawnId == 'M';
  if (!master && (pawnId < '0' || pawnId > '9')) {
    std::cout << std::format("Invalid pawn ID: {}!", pawnId) << std::endl;
    return std::nullopt;
  }

  if (cardNum >= HAND_SIZE) {
    std::cout << std::format("Pick a card number between 0 and {}!",
                             HAND_SIZE - 1)
              << std::endl;
    return std::nullopt;
  }

  const Game::Move move{.PawnId = (master ? size_t{0} : pawnId - '0'),
                        .UsedCard = game.GetCurrentHand()[cardNum],
                        .OffsetId = offsetNum};

  const std::optional<const std::string> errorMsg = game.IsInvalidMove(move);
  if (errorMsg) {
    std::cout << *errorMsg << std::endl;
    return std::nullopt;
  }

  return move;
}

std::optional<Game::Move> Human::ParseCard(const std::string& string,
                                           const Game::Game& game) {
  unsigned int cardNum;
  const size_t found = std::sscanf(string.c_str(), "%u", &cardNum);

  if (found != 1) {
    std::cout << "Failed to parse card number!" << std::endl;
    return std::nullopt;
  }

  if (cardNum >= HAND_SIZE) {
    std::cout << std::format("Pick a card number between 0 and {}!",
                             HAND_SIZE - 1)
              << std::endl;
    return std::nullopt;
  }

  return Game::Move{
      .PawnId = 0,
      .UsedCard = game.GetCurrentHand()[cardNum],
      .OffsetId = 0,
  };
}

}  // namespace Strategy
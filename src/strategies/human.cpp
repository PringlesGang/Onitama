#include "human.h"

#include <format>
#include <iostream>

#include "../constants.h"

namespace Strategy {

Game::Move Human::GetMove(const Game::Game& game) {
  std::optional<Game::Move> move;

  if (game.HasValidMoves()) {
    do {
      std::cout << std::endl
                << "Please enter your move (`{pawn ID} {card number} {offset "
                   "number}`):"
                << std::endl
                << "> ";

      std::string input;
      getline(std::cin, input);

      std::istringstream stream(std::move(input));
      move = ParseMove(stream, game);
    } while (!move);
  } else {
    do {
      std::cout << std::endl
                << "No valid moves. Please choose a card to discard:"
                << std::endl
                << "> ";

      std::string input;
      getline(std::cin, input);

      std::istringstream stream(std::move(input));
      move = ParseCard(stream, game);
    } while (!move);
  }

  return move.value();
}

std::optional<Game::Move> Human::ParseMove(std::istringstream& input,
                                           const Game::Game& game) {
  char pawnId;
  if (!(input >> pawnId)) {
    std::cout << "Failed to parse pawn ID!" << std::endl;
    return std::nullopt;
  }

  const bool master = pawnId == 'm' || pawnId == 'M';
  if (!master && (pawnId < '0' || pawnId >= '0' + game.GetPawnCount())) {
    std::cout << std::format("Invalid pawn ID \'{}\'!", pawnId) << std::endl;
    return std::nullopt;
  }

  size_t cardNum;
  if (!(input >> cardNum)) {
    std::cout << "Failed to parse card number!" << std::endl;
    return std::nullopt;
  }

  if (cardNum >= HAND_SIZE) {
    std::cout << std::format("Pick a card number between 0 and {}!",
                             HAND_SIZE - 1)
              << std::endl;
    return std::nullopt;
  }

  size_t offsetNum;
  if (!(input >> offsetNum)) {
    std::cout << "Failed to parse offset number!" << std::endl;
    return std::nullopt;
  }

  const size_t offsetCount = game.GetHand()[cardNum].GetMoves().size();
  if (offsetNum >= offsetCount) {
    std::cout << std::format("Pick an offset number between 0 and {}!",
                             offsetCount - 1)
              << std::endl;
    return std::nullopt;
  }

  const Game::Move move{.PawnId = (master ? size_t{0} : pawnId - '0'),
                        .UsedCard = game.GetHand()[cardNum],
                        .OffsetId = offsetNum};

  const std::optional<const std::string> errorMsg = game.IsInvalidMove(move);
  if (errorMsg) {
    std::cout << errorMsg.value() << std::endl;
    return std::nullopt;
  }

  return move;
}

std::optional<Game::Move> Human::ParseCard(std::istringstream& input,
                                           const Game::Game& game) {
  size_t cardNum;
  if (!(input >> cardNum)) {
    std::cout << "Failed to parse card number!" << std::endl;
    return std::nullopt;
  }

  if (cardNum >= HAND_SIZE) {
    std::cout << std::format("Pick a card number between 0 and {}!",
                             HAND_SIZE - 1)
              << std::endl;
    return std::nullopt;
  }

  return Game::Move{.UsedCard = game.GetHand()[cardNum]};
}

std::optional<std::function<std::unique_ptr<Human>()>> Human::Parse(
    std::istringstream& stream) {
  return std::make_unique<Human>;
}

}  // namespace Strategy
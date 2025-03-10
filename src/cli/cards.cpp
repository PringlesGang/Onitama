#include "cards.h"

#include <array>
#include <format>
#include <iostream>

namespace Cli {

void ExecuteCards() {
  const std::array<const std::string, (size_t)Game::CardType::CardTypeCount>
      cards = {
          "Boar",     "Cobra",  "Crab",    "Crane", "Dragon", "Eel",
          "Elephant", "Frog",   "Goose",   "Horse", "Mantis", "Monkey",
          "Ox",       "Rabbit", "Rooster", "Tiger",
      };
  for (size_t cardId = 0; cardId < (size_t)Game::CardType::CardTypeCount;
       cardId++) {
    std::cout << cards[cardId] << ':';
    Game::Card(Game::CardType(cardId)).Stream(std::cout, false) << std::endl;
  }
}

std::string CardsCommand::GetName() const { return "cards"; }

std::string CardsCommand::GetCommand() const { return GetName(); }

std::string CardsCommand::GetHelp() const {
  return "Prints all available cards.";
}

std::optional<Thunk> CardsCommand::Parse(std::istringstream& command) const {
  if (!Terminate(command)) return std::nullopt;

  return ExecuteCards;
}

}  // namespace Cli

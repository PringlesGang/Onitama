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

std::optional<Game::Card> ParseCard(std::istringstream& command) {
  std::string input;
  if (!(command >> input)) {
    std::cout << "Failed to parse card!" << std::endl;
    return std::nullopt;
  }
  Command::ToLower(input);

  using namespace Game;

  if (input == "boar") return Card(CardType::Boar);
  if (input == "cobra") return Card(CardType::Cobra);
  if (input == "crab") return Card(CardType::Crab);
  if (input == "crane") return Card(CardType::Crane);
  if (input == "dragon") return Card(CardType::Dragon);
  if (input == "eel") return Card(CardType::Eel);
  if (input == "elephant") return Card(CardType::Elephant);
  if (input == "frog") return Card(CardType::Frog);
  if (input == "goose") return Card(CardType::Goose);
  if (input == "horse") return Card(CardType::Horse);
  if (input == "mantis") return Card(CardType::Mantis);
  if (input == "monkey") return Card(CardType::Monkey);
  if (input == "ox") return Card(CardType::Ox);
  if (input == "rabbit") return Card(CardType::Rabbit);
  if (input == "rooster") return Card(CardType::Rooster);
  if (input == "tiger") return Card(CardType::Tiger);

  std::cout << std::format("Invalid card name \"{}\"!", input) << std::endl;
  return std::nullopt;
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

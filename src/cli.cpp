#include "cli.h"

#include <array>
#include <iostream>
#include <iterator>

#include "gameMaster.h"
#include "strategies/human.h"
#include "strategies/random.h"

namespace Cli {

constexpr std::string_view HELP_COMMAND = "help";
constexpr std::string_view GAME_COMMAND =
    "game red_strategy blue_strategy (--duplicate cards) "
    "(--repeat repeat_count) (--cards set_aside r1 r2 b1 b2)";
constexpr std::string_view STRATEGIES_COMMAND = "strategies";
constexpr std::string_view CARDS_COMMAND = "cards";
constexpr std::string_view EXIT_COMMAND = "exit";

void Help() {
  const std::array<const std::string_view, 5> commands = {
      HELP_COMMAND, GAME_COMMAND, STRATEGIES_COMMAND, CARDS_COMMAND,
      EXIT_COMMAND};

  for (const std::string_view& command : commands) {
    std::cout << std::format("- {}", command) << std::endl;
  }
}

void Game(const GameArgs args) {
  for (size_t game = 1; game <= args.RepeatCount; game++) {
    std::unique_ptr<GameMaster> master;

    if (args.Cards) {
      master = std::make_unique<GameMaster>(args.RedStrategy(),
                                            args.BlueStrategy(), *args.Cards);
    } else {
      master = std::make_unique<GameMaster>(
          args.RedStrategy(), args.BlueStrategy(), args.RepeatCards);
    }

    if (args.RepeatCount > 1) {
      std::cout << std::format("Game {}/{}:", game, args.RepeatCount)
                << std::endl;
    }

    do {
      master->Render();
      master->Update();
    } while (!master->IsFinished());
    master->Render();

    std::cout << std::endl << std::endl;
  }
}

void Strategies() {
  const std::array<const std::string, 2> strategies = {
      "human",
      "random",
  };

  for (const std::string& strategy : strategies) {
    std::cout << std::format("- {}", strategy) << std::endl;
  }
}

void Cards() {
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

static std::string ToLower(std::string input) {
  std::string lowered = "";
  for (char& character : input) lowered += tolower(character);
  return lowered;
}

Command Parse(const std::istringstream& command) {
  std::string option;

  std::istringstream lowered = std::istringstream(ToLower(command.str()));
  lowered >> option;

  if (option == "") {
    std::cout << "Failed to parse command!" << std::endl;
  } else {
    if (option == "help") return Help;
    if (option == "game") {
      Command parsed = ParseGame(lowered);
      if (!parsed) std::cout << GAME_COMMAND << std::endl;
      return parsed;
    }
    if (option == "strategies") return Strategies;
    if (option == "cards") return Cards;

    std::cout << std::format("Unknown command \"{}\"!", option) << std::endl;
  }

  return std::nullopt;
}

static Command ParseGame(std::istringstream& command) {
  GameArgs args{};
  if (!ParseGameOptionalArgs(command, args)) return std::nullopt;

  std::string redStrategyString;
  std::string blueStrategyString;

  command >> redStrategyString >> blueStrategyString;

  if (redStrategyString.empty()) {
    std::cout << "No strategy provided!" << std::endl;
    return std::nullopt;
  }
  if (blueStrategyString.empty()) {
    std::cout << "No blue strategy provided!" << std::endl;
    return std::nullopt;
  }

  const std::optional<StrategyFactory> redStrategy =
      ParseStrategy(redStrategyString);
  const std::optional<StrategyFactory> blueStrategy =
      ParseStrategy(blueStrategyString);

  if (!redStrategy || !blueStrategy) return std::nullopt;
  args.RedStrategy = *redStrategy;
  args.BlueStrategy = *blueStrategy;

  if (!ParseGameOptionalArgs(command, args)) return std::nullopt;

  std::string remaining;
  command >> remaining;
  if (!remaining.empty()) {
    std::cout << std::format("Unknown argument {}!", remaining) << std::endl;
    return std::nullopt;
  }

  return [args] { return Game(args); };
}

static bool ParseGameCards(std::istringstream& command, GameArgs& args) {
  args.Cards = std::array<Game::Card, CARD_COUNT>();
  for (Game::Card& card : *args.Cards) {
    std::string cardString;
    command >> cardString;

    std::optional<Game::Card> parsedCard = ParseCard(cardString);
    if (!parsedCard) return false;

    card = parsedCard.value();
  }

  return true;
}

static bool ParseGameOptionalArgs(std::istringstream& command, GameArgs& args) {
  std::string arg;
  command >> arg;

  if (arg.empty()) return true;

  if (arg == "--repeat" || arg == "-n") {
    if (!(command >> args.RepeatCount)) {
      std::cout << "Failed to parse repeat count!" << std::endl;
      return false;
    }
  } else if (arg == "--duplicate-cards" || arg == "-d") {
    args.RepeatCards = true;
  } else if (arg == "--cards" || arg == "-c") {
    if (!ParseGameCards(command, args)) return false;
  } else {
    // Un-parse the argument
    for (size_t i = 0; i < arg.size(); i++) command.unget();

    return true;
  }

  return ParseGameOptionalArgs(command, args);
}

std::optional<StrategyFactory> ParseStrategy(const std::string& name) {
  if (name == "human") {
    return [] { return std::make_unique<Strategy::Human>(); };
  }

  if (name == "random") {
    return [] { return std::make_unique<Strategy::Random>(); };
  }

  std::cout << std::format("Invalid strategy name \"{}\"!", name) << std::endl;
  return std::nullopt;
}

std::optional<Game::Card> ParseCard(const std::string& input) {
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

}  // namespace Cli
#include "cli.h"

#include <array>
#include <iostream>
#include <sstream>

#include "gameMaster.h"
#include "strategies/human.h"
#include "strategies/random.h"

namespace Cli {

void Help() {
  const std::array<const std::string, 5> commands = {
      "help",
      ("game red_strategy blue_strategy (--duplicate-cards) "
       "(--repeat repeat_count) (--cards set_aside r1 r2 b1 b2)"),
      "strategies",
      "cards",
      "exit",
  };

  for (const std::string& command : commands) {
    std::cout << std::format("- {}", command) << std::endl;
  }
}

void Game(const GameArgs args) {
  for (size_t game = 1; game <= args.Repeat; game++) {
    std::unique_ptr<GameMaster> master;

    if (args.Cards) {
      master = std::make_unique<GameMaster>(args.RedStrategy(),
                                            args.BlueStrategy(), *args.Cards);
    } else {
      master = std::make_unique<GameMaster>(
          args.RedStrategy(), args.BlueStrategy(), args.RepeatCards);
    }

    if (args.Repeat > 1) {
      std::cout << std::format("Game {}/{}:", game, args.Repeat) << std::endl;
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

Command Parse(const std::string& input) {
  std::string option;
  int read = input.size();

  std::string lowered = ToLower(input);
  const size_t matched =
      std::sscanf(lowered.c_str(), "%s %n", &option[0], &read);
  option = option.c_str();
  const std::string remaining = input.substr(read);

  if (matched != 0 && matched != EOF) {
    if (option == "help") return Help;
    if (option == "game") return ParseGame(remaining);
    if (option == "strategies") return Strategies;
    if (option == "cards") return Cards;

    std::cout << std::format("Unknown command \"{}\"!", option) << std::endl;
  } else {
    std::cout << "Failed to parse command!" << std::endl;
  }

  return std::nullopt;
}

static Command ParseGame(const std::string& input) {
  std::string argsParsedInput = input;
  GameArgs args{};
  if (!ParseGameOptionalArgs(argsParsedInput, args)) return std::nullopt;

  std::string redStrategyString;
  std::string blueStrategyString;
  int read = argsParsedInput.size();

  const size_t matched =
      std::sscanf(argsParsedInput.c_str(), "%s %s %n", &redStrategyString[0],
                  &blueStrategyString[0], &read);
  redStrategyString = redStrategyString.c_str();
  blueStrategyString = blueStrategyString.c_str();
  std::string remaining = input.substr(read);

  switch (matched) {
    case EOF:
    case 0:
      std::cout << "No strategy provided!" << std::endl;
      return std::nullopt;
    case 1:
      std::cout << "No blue strategy provided!" << std::endl;
      return std::nullopt;
    default:
      break;
  }

  const std::optional<StrategyFactory> redStrategy =
      ParseStrategy(redStrategyString);
  const std::optional<StrategyFactory> blueStrategy =
      ParseStrategy(blueStrategyString);

  if (!redStrategy || !blueStrategy) return std::nullopt;
  args.RedStrategy = *redStrategy;
  args.BlueStrategy = *blueStrategy;

  if (!ParseGameOptionalArgs(remaining, args)) return std::nullopt;

  return [args] { return Game(args); };
}

static bool ParseGameCards(std::string& input, GameArgs& args) {
  std::array<std::string, 5> cards;
  int read = input.size();

  const size_t matched = std::sscanf(input.c_str(), "%s %s %s %s %s %n",
                                     &cards[0][0], &cards[1][0], &cards[2][0],
                                     &cards[3][0], &cards[4][0], &read);
  for (std::string& card : cards) card = card.c_str();
  const std::string remaining = input.substr(read);

  if (matched < 5 || matched == EOF) {
    std::cout << "Did not provide enough cards!" << std::endl;
    return false;
  }

  args.Cards = std::array<Game::Card, 5>();
  auto cardsIt = args.Cards->begin();
  for (const std::string& card : cards) {
    const std::optional<Game::Card> parsedCard = ParseCard(card);

    if (!parsedCard) return false;
    *cardsIt = *parsedCard;
    cardsIt++;
  }

  return true;
}

static bool ParseGameOptionalArgs(std::string& input, GameArgs& args) {
  std::string arg;
  int read = input.size();

  const size_t matched = std::sscanf(input.c_str(), "%s %n", &arg[0], &read);
  arg = arg.c_str();
  std::string remaining = input.substr(read);

  if (matched == 0 || matched == EOF) return true;

  if (arg == "--repeat" || arg == "-n") {
    unsigned int count;

    const size_t matched2 =
        std::sscanf(remaining.c_str(), "%u %n", &count, &read);
    remaining = remaining.substr(read);

    if (matched2 == 0 || matched2 == EOF) {
      std::cout << "No repeat count provided!" << std::endl;
      return false;
    }

    args.Repeat = count;
    input = remaining;
  } else if (arg == "--duplicate-cards" || arg == "-d") {
    args.RepeatCards = true;
    input = remaining;
  } else if (arg == "--cards" || arg == "-c") {
    if (!ParseGameCards(remaining, args)) return false;
    input = remaining;
  }

  return ParseGameOptionalArgs(remaining, args);
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
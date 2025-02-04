#include "game.h"

#include <iostream>

#include "../gameMaster.h"
#include "cards.h"

namespace Cli {

void ExecuteGame(const GameArgs args) {
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

    master->PrintType = args.PrintType;

    do {
      master->Render();
      master->Update();
    } while (!master->IsFinished());
    master->Render();

    std::cout << std::endl << std::endl;
  }
}

std::string GameCommand::GetName() const { return "game"; }

std::string GameCommand::GetCommand() const {
  return std::format(
      "{} red_strategy blue_strategy "
      "(--duplicate-cards) "
      "(--repeat repeat_count) "
      "(--cards set_aside r1 r2 b1 b2) "
      "(--data)",
      GetName());
}

std::string GameCommand::GetHelp() const {
  return "Plays a game with the provided strategies.\n"

         "If `--repeat` is supplied, the game is played `repeat_count` times.\n"

         "With `--cards` the game cards can be specified. If not supplied, "
         "random cards will be chosen.\n"

         "By default, the random cards do not allow for repeats. "
         "`--duplicate-cards` suppresses this.\n"

         "Use `--data` to only print csv data.";
}

std::optional<Thunk> GameCommand::Parse(std::istringstream& command) const {
  GameArgs args{};
  if (!ParseOptionalArgs(command, args)) return std::nullopt;

  const std::optional<StrategyFactory> redStrategy = ParseStrategy(command);
  if (!redStrategy) return std::nullopt;

  const std::optional<StrategyFactory> blueStrategy = ParseStrategy(command);
  if (!blueStrategy) return std::nullopt;

  args.RedStrategy = redStrategy.value();
  args.BlueStrategy = blueStrategy.value();

  if (!ParseOptionalArgs(command, args)) return std::nullopt;

  if (!Terminate(command)) return std::nullopt;

  return [args] { return ExecuteGame(args); };
}

bool GameCommand::ParseCards(std::istringstream& command, GameArgs& args) {
  args.Cards = std::array<Game::Card, CARD_COUNT>();
  for (Game::Card& card : *args.Cards) {
    const std::optional<Game::Card> parsedCard = ParseCard(command);
    if (!parsedCard) return false;

    card = parsedCard.value();
  }

  return true;
}

bool GameCommand::ParseOptionalArgs(std::istringstream& command,
                                    GameArgs& args) {
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
    if (!ParseCards(command, args)) return false;
  } else if (arg == "--data" || arg == "-d") {
    args.PrintType = PrintType::Data;
  } else {
    Unparse(command, arg);
    return true;
  }

  return ParseOptionalArgs(command, args);
}

}  // namespace Cli

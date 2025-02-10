#include "game.h"

#include <format>
#include <iostream>

#include "../gameMaster.h"
#include "cards.h"

namespace Cli {

void ExecuteGame(const GameArgs args) {
  std::pair<size_t, size_t> wins;

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

    master->GameMasterPrintType = args.GameArgsPrintType;

    do {
      master->Render();
      master->Update();
    } while (!master->IsFinished());
    master->Render();

    if (master->IsFinished().value() == Color::Red) {
      wins.first++;
    } else {
      wins.second++;
    }

    std::cout << std::endl << std::endl;
  }

  if (args.GameArgsPrintType == PrintType::Wins) {
    std::cout << std::format("Red won {}/{} games; blue won {}/{} games.",
                             wins.first, args.RepeatCount, wins.second,
                             args.RepeatCount)
              << std::endl
              << std::endl;
  }
}

std::string GameCommand::GetName() const { return "game"; }

std::string GameCommand::GetCommand() const {
  return std::format(
      "{} red_strategy blue_strategy "
      "(--duplicate-cards) "
      "(--repeat repeat_count) "
      "(--cards set_aside r1 r2 b1 b2) "
      "(--print-type type)",
      GetName());
}

std::string GameCommand::GetHelp() const {
  return "Plays a game with the provided strategies.\n"

         "If `--repeat` is supplied, the game is played `repeat_count` times.\n"

         "With `--cards` the game cards can be specified. If not supplied, "
         "random cards will be chosen.\n"

         "By default, the random cards do not allow for repeats. "
         "`--duplicate-cards` suppresses this.\n"

         "Use `--print-type` to specify in what output the game should give.";
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

bool GameCommand::ParsePrintType(std::istringstream& command, GameArgs& args) {
  std::string arg;

  if (!(command >> arg)) {
    std::cout << "Failed to parse print type!" << std::endl;
    return false;
  }

  if (arg == "board") {
    args.GameArgsPrintType = PrintType::Board;
  } else if (arg == "data") {
    args.GameArgsPrintType = PrintType::Data;
  } else if (arg == "wins") {
    args.GameArgsPrintType = PrintType::Wins;
  } else if (arg == "none") {
    args.GameArgsPrintType = PrintType::None;
  } else {
    std::cout << std::format("Unknown print type \"{}\"", arg) << std::endl;
    return false;
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
  } else if (arg == "--print-type" || arg == "-p") {
    if (!ParsePrintType(command, args)) return false;
  } else {
    Unparse(command, arg);
    return true;
  }

  return ParseOptionalArgs(command, args);
}

}  // namespace Cli

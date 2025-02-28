#include "game.h"

#include <format>
#include <future>
#include <iostream>

#include "../gameMaster.h"
#include "cards.h"

namespace Cli {

static Color RunGame(const GameArgs args, std::ostream* stream) {
  std::unique_ptr<GameMaster> master;

  if (args.Cards) {
    master = std::make_unique<GameMaster>(args.Width, args.Height,
                                          args.RedStrategy(),
                                          args.BlueStrategy(), *args.Cards);
  } else {
    master = std::make_unique<GameMaster>(
        args.Width, args.Height, args.RedStrategy(), args.BlueStrategy(),
        args.RepeatCards);
  }

  master->GameMasterPrintType = args.GameArgsPrintType;

  do {
    master->Render(*stream);
    master->Update();
  } while (!master->IsFinished());
  master->Render(*stream);

  return master->IsFinished().value();
}

ExecuteGameInfo ExecuteGame(const GameArgs args) {
  ExecuteGameInfo info{};
  const bool print = args.GameArgsPrintType != PrintType::None;

  if (args.Multithread) {
    std::vector<std::stringstream> streams;
    std::vector<std::future<Color>> futures;
    streams.resize(args.RepeatCount);
    futures.reserve(args.RepeatCount);

    for (size_t game = 0; game < args.RepeatCount; game++) {
      futures.emplace_back(
          std::async(std::launch::async, RunGame, args, &streams[game]));
    }

    for (size_t game = 0; game < args.RepeatCount; game++) {
      if (futures[game].get() == Color::Red) {
        info.Wins.first++;
      } else {
        info.Wins.second++;
      }

      if (print) {
        if (args.RepeatCount > 1) {
          std::cout << std::format("Game {}/{}:", game + 1, args.RepeatCount)
                    << std::endl;
        }
        std::cout << streams[game].str() << std::endl;
      }
    }
  } else {
    for (size_t game = 1; game <= args.RepeatCount; game++) {
      if (print && args.RepeatCount > 1) {
        std::cout << std::format("Game {}/{}:", game, args.RepeatCount)
                  << std::endl;
      }

      const Color winner = RunGame(args, &std::cout);

      if (winner == Color::Red) {
        info.Wins.first++;
      } else {
        info.Wins.second++;
      }

      if (print) std::cout << std::endl;
    }
  }

  if (args.GameArgsPrintType == PrintType::Wins) {
    std::cout << std::format("Red won {}/{} games; blue won {}/{} games.",
                             info.Wins.first, args.RepeatCount,
                             info.Wins.second, args.RepeatCount)
              << std::endl
              << std::endl;
  }

  return info;
}

std::string GameCommand::GetName() const { return "game"; }

std::string GameCommand::GetCommand() const {
  return std::format(
      "{} red_strategy blue_strategy "
      "(--duplicate-cards) "
      "(--repeat repeat_count) "
      "(--cards set_aside r1 r2 b1 b2) "
      "(--print-type type) "
      "(--multithread) "
      "(--size width height)",
      GetName());
}

std::string GameCommand::GetHelp() const {
  return "Plays a game with the provided strategies.\n"

         "If `--repeat` is supplied, the game is played `repeat_count` times.\n"
         "With --multithread these games are run on different threads.\n"

         "With `--cards` the game cards can be specified. If not supplied, "
         "random cards will be chosen.\n"

         "By default, the random cards do not allow for repeats. "
         "`--duplicate-cards` suppresses this.\n"

         "Use `--print-type` to specify in what output the game should give.\n"

         "The size of the board can be altered using --size, "
         "with a default of 5x5.";
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

  } else if (arg == "--multithread" || arg == "-m") {
    args.Multithread = true;

  } else if (arg == "--size" || arg == "-s") {
    if (!(command >> args.Width)) {
      std::cout << "Failed to parse board width!" << std::endl;
      return false;
    }

    if (!(command >> args.Height)) {
      std::cout << "Failed to parse board height!" << std::endl;
      return false;
    }

    if (args.Width < 1 || args.Height < 2) {
      std::cout << "The board needs to be at least 1x2!" << std::endl;
      return false;
    }

  } else {
    Unparse(command, arg);
    return true;
  }

  return ParseOptionalArgs(command, args);
}

}  // namespace Cli

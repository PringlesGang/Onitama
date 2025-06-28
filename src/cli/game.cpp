#include "game.h"

#include <format>
#include <future>
#include <iostream>

#include "../gameMaster.h"
#include "cards.h"

namespace Cli {

static Color RunGame(const GameArgs args, std::ostream* stream) {
  if (!args.IsValid())
    throw std::invalid_argument("Invalid arguments for game!");

  std::unique_ptr<GameMaster> master;

  master =
      std::make_unique<GameMaster>(args.Configuration.ToGame().value(),
                                   args.RedStrategy(), args.BlueStrategy());
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

std::optional<Thunk> GameCommand::Parse(std::istringstream& command) const {
  if (Parse::ParseHelp(command))
    return [this] { std::cout << GetHelp() << std::endl; };

  GameArgs args;
  if (!args.Parse(command)) return std::nullopt;

  const std::optional<StrategyFactory> redStrategy = ParseStrategy(command);
  if (!redStrategy) return std::nullopt;

  const std::optional<StrategyFactory> blueStrategy = ParseStrategy(command);
  if (!blueStrategy) return std::nullopt;

  args.RedStrategy = redStrategy.value();
  args.BlueStrategy = blueStrategy.value();

  if (!args.Parse(command)) return std::nullopt;

  if (!Terminate(command)) return std::nullopt;
  if (!args.IsValid()) return std::nullopt;

  return [args] { return ExecuteGame(args); };
}

bool GameArgs::Parse(std::istringstream& stream) {
  if (!Configuration.Parse(stream)) return false;

  std::string arg;
  stream >> arg;
  Parse::ToLower(arg);

  if (arg.empty()) return true;

  if (arg == "--repeat" || arg == "-n") {
    if (!(stream >> RepeatCount)) {
      std::cerr << "Failed to parse repeat count!" << std::endl;
      return false;
    }

  } else if (arg == "--print-type" || arg == "-p") {
    std::string printTypeString;
    stream >> printTypeString;

    const std::optional<PrintType> printType = ParsePrintType(printTypeString);
    if (!printType) return false;
    GameArgsPrintType = printType.value();

  } else if (arg == "--multithread" || arg == "-m") {
    Multithread = true;

  } else {
    Parse::Unparse(stream, arg);
    return true;
  }

  return Parse(stream);
}

bool GameArgs::IsValid() const {
  if (!Configuration.IsValid()) return false;

  if (RedStrategy == nullptr || BlueStrategy == nullptr) return false;

  return true;
}

}  // namespace Cli

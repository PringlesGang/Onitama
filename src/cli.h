#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "game/card.h"
#include "strategies/strategy.h"

namespace Cli {

typedef std::optional<std::function<void()>> Command;
typedef std::function<std::unique_ptr<Strategy::Strategy>()> StrategyFactory;

struct GameArgs {
  StrategyFactory RedStrategy;
  StrategyFactory BlueStrategy;

  size_t Repeat = 1;
  bool RepeatCards = false;

  std::optional<std::array<Game::Card, CARD_COUNT>> Cards;
};

void Help();
void Game(const GameArgs args);

static std::string ToLower(std::string input);

Command Parse(const std::string& input);

static Command ParseGame(const std::string& input);
static bool ParseGameOptionalArgs(std::string& input, GameArgs& args);
static bool ParseGameCards(std::string& input, GameArgs& args);

std::optional<StrategyFactory> ParseStrategy(const std::string& input);
std::optional<Game::Card> ParseCard(const std::string& input);

}  // namespace Cli

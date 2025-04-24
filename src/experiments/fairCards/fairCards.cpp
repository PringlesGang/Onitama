#include "fairCards.h"

#include <format>

#include "../../cli/game.h"
#include "../../strategies/random.h"

namespace Experiments {
namespace FairCards {

static void AddCombinations(
    std::vector<std::array<Game::Card, CARD_COUNT>>& vector,
    std::array<Game::Card, CARD_COUNT> currentCards, size_t index) {
  if (index == CARD_COUNT) {
    vector.emplace_back(std::move(currentCards));
    return;
  }

  const size_t firstCard =
      index == 0 ? 0 : (size_t)currentCards[index - 1].Type + 1;
  if (firstCard == (size_t)Game::CardType::CardTypeCount) return;

  for (size_t newCard = firstCard;
       newCard < (size_t)Game::CardType::CardTypeCount; newCard++) {
    currentCards[index] = Game::Card(Game::CardType(newCard));
    AddCombinations(vector, currentCards, index + 1);
  }
}

static std::vector<std::array<Game::Card, CARD_COUNT>> GetCombinations() {
  std::vector<std::array<Game::Card, CARD_COUNT>> combinations;
  AddCombinations(combinations, std::array<Game::Card, CARD_COUNT>(), 0);

  return std::move(combinations);
}

void Execute(const size_t repeatCount, const Cli::StrategyFactory strategy) {
  const std::vector<std::array<Game::Card, CARD_COUNT>> combinations =
      GetCombinations();

  // Print header
  std::cout << std::format("Fair cards experiment with repeat count {}:",
                           repeatCount)
            << std::endl;
  std::cout << "co. num, co. count, ";
  for (size_t i = 1; i <= CARD_COUNT; i++)
    std::cout << std::format("crd{}, ", i);
  std::cout << "red wins, blue wins" << std::endl;

  size_t combinationNumber = 0;
  for (std::array<Game::Card, CARD_COUNT> cards : combinations) {
    std::cout << std::format("{:04d}, {}, ", ++combinationNumber,
                             combinations.size());

    for (Game::Card card : cards) {
      std::cout << std::format("{}, ", card.GetName());
    }

    Cli::GameArgs args{
        .RedStrategy = strategy,
        .BlueStrategy = strategy,

        .Configuration{
            .Cards = cards,
        },

        .RepeatCount = repeatCount,
        .Multithread = true,

        .GameArgsPrintType = PrintType::None,
    };

    const Cli::ExecuteGameInfo info = Cli::ExecuteGame(args);

    std::cout << std::format("{}, {}", info.Wins.first, info.Wins.second)
              << std::endl;
  }
}

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  size_t repeatCount;
  if (!(command >> repeatCount)) {
    std::cout << "Did not provide valid repeat count for fair cards experiment!"
              << std::endl;
    return std::nullopt;
  }

  if (repeatCount == 0) {
    std::cout << "Fair card experiment repeat count must be greater than 0!"
              << std::endl;
    return std::nullopt;
  }

  const std::optional<Cli::StrategyFactory> strategy =
      Cli::ParseStrategy(command);
  if (!strategy) return std::nullopt;

  return [repeatCount, strategy] { return Execute(repeatCount, *strategy); };
}

}  // namespace FairCards
}  // namespace Experiments

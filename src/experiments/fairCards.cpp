#include "fairCards.h"

#include "../cli/game.h"
#include "../strategies/random.h"

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

  return combinations;
}

void Execute(size_t repeatCount) {
  std::vector<std::array<Game::Card, CARD_COUNT>> combinations =
      GetCombinations();

  size_t combinationNumber = 0;
  for (std::array<Game::Card, CARD_COUNT> cards : combinations) {
    std::cout << std::format("Combination {}/{}:", ++combinationNumber,
                             combinations.size())
              << std::endl;

    for (Game::Card card : cards) {
      std::cout << std::format("{}, ", card.GetName());
    }
    std::cout << std::endl << std::endl;

    Cli::GameArgs args{
        .RedStrategy = [] { return std::make_unique<Strategy::Random>(); },
        .BlueStrategy = [] { return std::make_unique<Strategy::Random>(); },

        .RepeatCount = repeatCount,
        .Multithread = true,

        .GameArgsPrintType = PrintType::Wins,

        .Cards = cards,
    };

    Cli::ExecuteGame(args);

    std::cout << "========================================================="
              << std::endl
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

  return [repeatCount] { return Execute(repeatCount); };
}

}  // namespace FairCards
}  // namespace Experiments

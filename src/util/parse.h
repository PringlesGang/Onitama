#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>

#include "../constants.h"
#include "../game/card.h"
#include "../game/game.h"

namespace Parse {

void Unparse(std::istringstream& stream, const std::string& string);
void ToLower(std::string& string);
bool Terminate(std::istringstream& stream);
bool ParseHelp(std::istringstream& stream);

constexpr std::string PadCommandName(
    const std::string_view name, const std::span<const std::string_view> help) {
  constexpr size_t HelpPaddingSize = 24;
  const size_t nameLength = name.size();

  std::string string = std::string(name);
  if (nameLength >= HelpPaddingSize - 2) {
    string +=
        "\n" + std::string(HelpPaddingSize, ' ') + std::string(help[0]) + "\n";
  } else {
    string += std::string(HelpPaddingSize - nameLength, ' ') +
              std::string(help[0]) + "\n";
  }

  for (size_t i = 1; i < help.size(); i++) {
    string += std::string(HelpPaddingSize, ' ') + std::string(help[i]) + "\n";
  }

  return string;
}

constexpr std::string PadCommandName(const std::string_view name,
                                     const std::string_view description) {
  return PadCommandName(name, std::array<std::string_view, 1>{description});
}

std::optional<std::filesystem::path> ParsePath(std::istringstream& stream);

std::optional<std::array<Game::Card, CARD_COUNT>> ParseCards(
    std::istringstream& stream);

std::optional<std::pair<size_t, size_t>> ParseDimensions(
    std::istringstream& stream);

struct GameConfiguration {
  bool Parse(std::istringstream& stream);
  bool IsValid() const;

  std::pair<size_t, size_t> Dimensions = {5, 5};

  bool RepeatCards = false;
  std::optional<std::array<Game::Card, CARD_COUNT>> Cards;

  std::optional<Game::Game> ToGame() const;
};

}  // namespace Parse

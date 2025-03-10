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

#include "parse.h"

#include <iostream>

namespace Parse {

void Unparse(std::istringstream& stream, const std::string& string) {
  for (size_t i = 0; i < string.size(); i++) {
    stream.unget();
  }
}

void ToLower(std::string& string) {
  for (char& character : string) character = tolower(character);
}

bool Terminate(std::istringstream& stream) {
  std::string remaining;
  stream >> remaining;

  if (remaining.empty()) return true;

  Unparse(stream, remaining);
  return false;
}

std::optional<std::filesystem::path> ParsePath(std::istringstream& stream) {
  std::string path;
  stream >> path;

  if (path.empty()) {
    std::cerr << "No filepath provided!" << std::endl;
    return std::nullopt;
  }

  try {
    return std::filesystem::path(path);
  } catch (std::exception e) {
    std::cerr << "Failed to construct filepath: " << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::array<Game::Card, CARD_COUNT>> ParseCards(
    std::istringstream& stream) {
  std::array<Game::Card, CARD_COUNT> cards;
  for (Game::Card& card : cards) {
    const std::optional<Game::Card> parsedCard = Game::Card::Parse(stream);
    if (!parsedCard) return std::nullopt;

    card = parsedCard.value();
  }

  return cards;
}

std::optional<std::pair<size_t, size_t>> ParseDimensions(
    std::istringstream& stream) {
  std::pair<size_t, size_t> dimensions;

  if (!(stream >> dimensions.first)) {
    std::cerr << "Failed to parse width!" << std::endl;
    return std::nullopt;
  }

  if (!(stream >> dimensions.second)) {
    std::cerr << "Failed to parse height!" << std::endl;
    return std::nullopt;
  }

  return dimensions;
}

bool GameConfiguration::Parse(std::istringstream& stream) {
  std::string arg;
  stream >> arg;
  ToLower(arg);

  if (arg.empty()) return true;

  if (arg == "--duplicate-cards" || arg == "-d") {
    RepeatCards = true;

  } else if (arg == "--cards" || arg == "-c") {
    Cards = ParseCards(stream);
    if (!Cards) return false;

  } else if (arg == "--size" || arg == "-s") {
    const std::optional<std::pair<size_t, size_t>> dimensions =
        ParseDimensions(stream);
    if (!dimensions) return false;
    Dimensions = dimensions.value();

  } else {
    Unparse(stream, arg);
    return true;
  }

  return Parse(stream);
}

bool GameConfiguration::IsValid() const {
  if (Dimensions.first < 1 || MAX_DIMENSION < Dimensions.first) return false;
  if (Dimensions.second < 2 || MAX_DIMENSION < Dimensions.second) return false;

  if (Cards) {
    for (Game::Card card : Cards.value()) {
      if (card.Type == Game::CardType::CardTypeCount) return false;
    }
  }

  return true;
}

std::optional<Game::Game> GameConfiguration::ToGame() const {
  if (!IsValid()) return std::nullopt;

  if (Cards)
    return Game::Game(Dimensions.first, Dimensions.second, Cards.value());

  return Game::Game::WithRandomCards(Dimensions.first, Dimensions.second,
                                     RepeatCards);
}

}  // namespace Parse

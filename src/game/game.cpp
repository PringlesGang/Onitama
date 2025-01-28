#include "game.h"

#include <random>

Game::Game(std::array<Card, CARD_COUNT> cards)
    : Cards(cards), Board(), CurrentPlayer(cards[0].GetColor()) {}

Game Game::WithRandomCards(bool repeatCards) {
  std::array<Card, CARD_COUNT> cards;

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<size_t> randomCard(
      0, (size_t)CardType::CardTypeCount);

  for (auto cardIt = cards.begin(); cardIt != cards.end(); cardIt++) {
    do {
      *cardIt = Card(CardType(randomCard(generator)));
    } while (!repeatCards &&
             std::find_if(cards.begin(), cardIt, [cardIt](Card& card) {
               return card.Type == cardIt->Type;
             }) != cardIt);
  }

  return Game(cards);
}

std::optional<Color> Game::IsFinished() const { return Board.IsFinished(); }

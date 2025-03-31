#pragma once

namespace Tests {
namespace Game {

void Init();

int InitialStateConstructor();
int BoardConstructor();
int CopyConstructor();
int MoveConstructor();
int WithRandomCards();
int FromSerialization();

int ParseSerialization();
int ParseIncorrectSerialization();

int Equality();
int Inequality();

int GetHand();
int GetCurrentHand();
int GetSetAsideCard();
int GetCurrentPlayer();
int GetDimensions();

}  // namespace Game
}  // namespace Tests

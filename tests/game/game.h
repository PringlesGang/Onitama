#pragma once

namespace Tests {
namespace Game {
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

int GetPawnCoordinates();
int GetCurrentPawnCoordinates();
int MasterCaptured();
int CurrentMasterCaptured();
int GetPawnCount();
int GetCurrentPawnCount();

int GetValidMoves();
int HasValidMoves();
int IsValidMove();

int IsFinished();
int DoMove();

int Serialize();

}  // namespace Game
}  // namespace Game
}  // namespace Tests

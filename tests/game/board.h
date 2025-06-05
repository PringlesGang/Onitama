#pragma once

namespace Tests {
namespace Game {
namespace Board {

void Init();

int InitialStateConstructorEvenWidth();
int InitialStateConstructorOddWidth();
int GridConstructor();
int CopyConstructor();
int MoveConstructor();

int EqualityOperator();

int Reset();
int DoMove();

int GetGrid();
int GetTile();
int GetRow();
int GetPawnCoordinates();
int GetDimensions();
int GetTemple();
int MasterCaptured();

int OnBoard();
int IsFinished();

}  // namespace Board
}  // namespace Game
}  // namespace Tests
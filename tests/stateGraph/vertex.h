#pragma once

namespace Tests {
namespace StateGraph {
namespace Vertex {

void Init();

int GameConstructor();
int SerializationConstructor();

int EqualityOperator();
int GameEqualityOperator();

int SetOptimalMove();
int GetOptimalMove();

int GetEdgeMove();
int GetEdgeSerialization();

}  // namespace Vertex
}  // namespace StateGraph
}  // namespace Tests

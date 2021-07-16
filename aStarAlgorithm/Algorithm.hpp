#pragma once
#include "Graph.hpp"
class Algorithm {
public:
	static int currentIteration;
	static bool findPath(int* o_XPos, int* o_YPos, int* o_pathLenght, Graph* graph, int startRow, int startCol, int goalRow, int goalCol);
private:
	static float getHeuristic(int* currentGraph, int* xPositions, int* yPositions, int startIndex, int goalIndex);
};
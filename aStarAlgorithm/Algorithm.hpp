#pragma once
#include "Graph.hpp"
class Algorithm {
public:
	static int currentIteration;
	static bool findPath(int* o_XPos, int* o_YPos, int* o_pathLenght, Graph* graph, unsigned int startIndex, unsigned int goalIndex);
private:
	static float getHeuristic(int* currentGraph, int* xPositions, int* yPositions, int startIndex, int goalIndex);
};
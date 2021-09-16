#pragma once
#include "Graph.hpp"
class Algorithm {
public:
	static int currentIteration;
	static bool findPath(shared_ptr<int[]>&& o_pathYs, shared_ptr<int[]>&& o_pathXs, int&& o_pathLenght, Graph* graph, int startY, int startX, int goalY, int goalX);
private:
	static float getHeuristic(int* currentGraph, int* ys, int* xs, int startIndex, int goalIndex);
};
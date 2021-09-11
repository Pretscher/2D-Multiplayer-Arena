#pragma once
#include "Graph.hpp"
class Algorithm {
public:
	static int currentIteration;
	static bool findPath(int** o_pathRows, int** o_pathCols, int* o_pathLenght, Graph* graph, int startRow, int startCol, int goalRow, int goalCol);
private:
	static float getHeuristic(int* currentGraph, int* rows, int* cols, int startIndex, int goalIndex);
};
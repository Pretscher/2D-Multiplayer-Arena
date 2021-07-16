#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "GraphNode.hpp"

class Graph {
public:
	Graph(int rows, int cols);
	int* xCoords;
	int* yCoords;
	int* neighbourCount;
	int** neighbourCosts;
	int* heapIndices;
	int** neighbourIndices;
	int* currentGraph;
	int graphNodeCount;
	int** rawIndices;
	//for getIndexFromCoords() method
	int rows;
	int cols;

	void generateWorldGraph(bool** isUseable);
	int getIndexFromCoords(int row, int col);
	~Graph();
};

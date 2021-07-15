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
	bool** isUseable;
	int* currentGraph;
	int graphNodeCount;
	int** rawIndices;
	//for getIndexFromCoords() method
	int rows;
	int cols;

	void generateWorldGraph(bool** isUseable);

	~Graph() {
		delete[] xCoords;
		delete[] yCoords;
		delete[] neighbourCount;
		delete[] heapIndices;
		delete[] currentGraph;
		for (int i = 0; i < graphNodeCount; i++) {
			delete[] neighbourCosts[i];
			delete[] neighbourIndices[i];
			delete[] rawIndices[i];
		}
		delete[] neighbourCosts;
		delete[] neighbourIndices;
		delete[] rawIndices;
	}
};

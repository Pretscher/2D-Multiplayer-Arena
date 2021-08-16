#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "GraphNode.hpp"

class Graph {
public:
	Graph(int rows, int cols, float i_accuracy);
	float accuracy;

	bool* usedByMoveable;
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
	int getIndexFromCoords(int row, int col, bool moveableRelevant);
	~Graph();
	void reset();
	std::vector<int>* blockedX;
	std::vector<int>* blockedY;
	void disableObjectBounds(int row, int col, int width, int height);
	void moveObject(int row, int col, int oldRow, int oldCol, int width, int height);
	void enableObjectBounds(int row, int col, int width, int height);

	//debugging

	std::vector<int> deactivatedX = std::vector<int>();
	std::vector<int> deactivatedY = std::vector<int>();
	void debugDrawing();
private:
	int findNextUseableVertex(int row, int col, bool moveableRelevant);
};

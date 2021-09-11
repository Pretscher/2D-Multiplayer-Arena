#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "GraphNode.hpp"
#include <mutex>

class Graph {
public:
	Graph(int rows, int cols, float i_accuracy);

	float accuracy;

	bool* usedByMoveable;

	int* neighbourCount;
	int** neighbourCosts;
	int* heapIndices;
	int** neighbourIndices;
	int* currentGraph;
	int graphNodeCount;
	int** rawIndices;

	int* getIndexBoundRows() {
		return indexBoundRows;
	}

	int* getIndexBoundCols() {
		return indexBoundCols;
	}



	void generateWorldGraph(bool** isUseable);
	int getIndexFromCoords(int row, int col, bool moveableRelevant);
	~Graph();
	void reset();
	void disableObjectBounds(int row, int col, int width, int height);
	void moveObject(int row, int col, int oldRow, int oldCol, int width, int height);
	void enableObjectBounds(int row, int col, int width, int height);

	//debugging

	std::vector<int> deactivatedX;
	std::vector<int> deactivatedY;
	void debugDrawing();

	void findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant);
	bool isDisabled(int index) {
		return usedByMoveable[index];
	}

private:
	int findNextUseableVertex(int row, int col, bool moveableRelevant);

	//for getIndexFromCoords() and getting coords of indices
	int rowCount;
	int colCount;
	int* indexBoundCols;
	int* indexBoundRows;
};

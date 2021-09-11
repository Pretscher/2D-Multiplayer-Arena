#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <mutex>

class Graph {
public:
	Graph(int ys, int xs, float i_accuracy);

	void generateWorldGraph(bool** isUseable);
	int getIndexFromCoords(int y, int x, bool moveableRelevant);
	~Graph();
	void disableObjectBounds(int y, int x, int width, int height);
	void moveObject(int y, int x, int oldY, int oldX, int width, int height);
	void enableObjectBounds(int y, int x, int width, int height);

	//debugging
	std::vector<int> deactivatedX;
	std::vector<int> deactivatedY;
	void debugDrawing();


	void findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant);
	bool isDisabled(int index) {
		return usedByMoveable[index];
	}

	int* getIndexBoundYs() {
		return indexBoundYs;
	}

	int* getIndexBoundXs() {
		return indexBoundXs;
	}

	int** getIndexNeighbourCosts() {
		return neighbourCosts;
	}

	int** getNeighbourIndices() {
		return neighbourIndices;
	}

	int* getIndexNeighbourCount() {
		return neighbourCount;
	}

	int* getHeapIndices() {
		return heapIndices;
	}

	int getGraphNodeCount() {
		return graphNodeCount;
	}

	int* getGraph() {
		return currentGraph;
	}

	bool* isUsedByMoveableObject() {
		return usedByMoveable;
	}

	void resetHeapIndices() {
		for (int nodeIndex = 0; nodeIndex < graphNodeCount; nodeIndex++) {
			heapIndices[nodeIndex] = -1;
		}
	}

	void setNeighbourCost(int nodeIndex, int neighbourIndex, int cost) {
		neighbourCosts[nodeIndex][neighbourIndex] = cost;
	}

private:
	int findNextUseableVertex(int y, int x, bool moveableRelevant);

	//for getIndexFromCoords() and getting coords of indices
	int yCount;
	int xCount;
	int* indexBoundXs;
	int* indexBoundYs;



	float accuracy;
	bool* usedByMoveable;
	int* neighbourCount;
	int** neighbourCosts;
	int* heapIndices;
	int** neighbourIndices;
	int* currentGraph;
	int graphNodeCount;
	int** rawIndices;
};

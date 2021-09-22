#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;
#include <mutex>
#include <memory>

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
	vector<int> deactivatedX;
	vector<int> deactivatedY;
	void debugDrawing();


	void findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant);
	bool isDisabled(int index) {
		return usedByMoveable[index];
	}

	shared_ptr<int[]> getIndexBoundYs() {
		return indexBoundYs;
	}

	shared_ptr<int[]> getIndexBoundXs() {
		return indexBoundXs;
	}

	int** getIndexNeighbourCosts() {
		return neighbourCosts;
	}

	int** getNeighbourIndices() {
		return neighbourIndices;
	}


	int getGraphNodeCount() {
		return graphNodeCount;
	}

	shared_ptr<int[]> getIndexNeighbourCount() {
		return neighbourCount;
	}

	shared_ptr<int[]> getHeapIndices() {
		return heapIndices;
	}

	shared_ptr<bool[]> isUsedByMoveableObject() {
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
	float accuracy;

	int** rawIndices;

	int** neighbourCosts;
	int** neighbourIndices;

	int graphNodeCount;//lenght for all of these arrays
	shared_ptr<bool[]> usedByMoveable;
	shared_ptr<int[]> indexBoundXs;
	shared_ptr<int[]> indexBoundYs;
	shared_ptr<int[]> neighbourCount;
	shared_ptr<int[]> heapIndices;
};

#include "BinaryHeap.hpp"
#include <cmath>
#include <chrono>
#include "Algorithm.hpp"

int Algorithm::currentIteration = -1;
bool Algorithm::findPath(int** o_XPos, int** o_YPos, int* o_pathLenght, Graph* graph, int startRow, int startCol, int goalRow, int goalCol) {
	currentIteration++;
	
	int startIndex = graph->getIndexFromCoords(startRow, startCol);
	int goalIndex = graph->getIndexFromCoords(goalRow, goalCol);

	int graphNodeCount = graph->graphNodeCount;
	//initialize GraphnodeHeuristics
	graph->neighbourCosts = new int*[graphNodeCount];
	for (int nodeIndex = 0; nodeIndex < graphNodeCount; nodeIndex++) {
		graph->heapIndices[nodeIndex] = -1;
		graph->neighbourCosts[nodeIndex] = new int[graph->neighbourCount[nodeIndex]];
		for (int i = 0; i < graph->neighbourCount[nodeIndex]; i++) {
			int currentNeighbourIndex = graph->neighbourIndices[nodeIndex][i];
			float heuristics = getHeuristic(graph->currentGraph, graph->xCoords, graph->yCoords, currentNeighbourIndex, goalIndex);
			graph->neighbourCosts[nodeIndex][i] = heuristics;
		}
	}

	float* distanceTravelled = new float[graphNodeCount];
	int* previousIndex = new int[graphNodeCount];

	for (int i = 0; i < graphNodeCount; i++) {
		distanceTravelled[i] = std::numeric_limits<float>::max();//Theoretically infinity, not used nodes have no valuable info
	}

	distanceTravelled[startIndex] = 0.0f;
	previousIndex[startIndex] = startIndex;
	BinaryHeap* heap = new BinaryHeap(graph, graphNodeCount);
	//insert start node with the value 0
	HeapNode* toInsert = new HeapNode(getHeuristic(graph->currentGraph, graph->xCoords, graph->yCoords,
		graph->currentGraph[startIndex], graph->currentGraph[goalIndex]), startIndex);
	heap->insert(toInsert);
	bool foundPath = false;

	while (heap->getCurrentNodeCount() > 0) {//while heap is not empty
		HeapNode* helpNode = heap->extractMin();//extract best node
		int cNodeIndex = graph->currentGraph[helpNode->getIndexInGraph()];//get graphIndex of best node
		int neighbourCount = graph->neighbourCount[cNodeIndex];//we will look through graph->neighbourIndices of this node
		if (cNodeIndex == goalIndex) {
			foundPath = true;
			break;
		}

		for (int i = 0; i < neighbourCount; i++) {
			int cNeighbourIndex = graph->neighbourIndices[cNodeIndex][i];
			float tempDistance = distanceTravelled[cNodeIndex] + graph->neighbourCosts[cNodeIndex][i];

			if (tempDistance < distanceTravelled[cNeighbourIndex]) {
				distanceTravelled[cNeighbourIndex] = tempDistance;
				previousIndex[cNeighbourIndex] = cNodeIndex;

				float heuristicOfCurrentNeighbour = tempDistance + getHeuristic(graph->currentGraph, graph->xCoords,
					graph->yCoords, cNeighbourIndex, goalIndex);

				bool alreadyInserted = true;
				//if graphnode has been inserted to heap (index in heap initialized to -1)
				if (graph->heapIndices[cNeighbourIndex] == -1) {
					alreadyInserted = false;
				}

				if (alreadyInserted == true) {
					heap->decrease(graph->heapIndices[cNeighbourIndex], heuristicOfCurrentNeighbour);
				} else {
					HeapNode* nodeToInsert = new HeapNode(heuristicOfCurrentNeighbour, cNeighbourIndex);
					heap->insert(nodeToInsert);
				}
			}
		}
		delete helpNode;
	}

	if (foundPath == true) {

		//std::cout << "time elapsed sind the algorithm started: " << Utils::endTimerGetTime();

		//get lenght of path array
		int pathLenght = 0;
		int currentIndex = goalIndex;
		while (currentIndex != startIndex) {
			currentIndex = previousIndex[currentIndex];
			pathLenght++;
		}
		//put path indices into path array from end to front

		*o_XPos = new int[pathLenght];
		*o_YPos = new int[pathLenght];
		int indexInPath = pathLenght;

		currentIndex = goalIndex;
		while (currentIndex != startIndex) {
			(*o_XPos)[indexInPath] = graph->xCoords[currentIndex];
			(*o_YPos)[indexInPath] = graph->yCoords[currentIndex];
			currentIndex = previousIndex[currentIndex];
			indexInPath--;
		}
		(*o_XPos)[0] = graph->xCoords[startIndex];
		(*o_YPos)[0] = graph->yCoords[startIndex];
		*o_pathLenght = pathLenght;
		std::cout << "path found";

		delete heap;
		delete[] distanceTravelled;
		delete[] previousIndex;
		return true;
	}
	delete heap;
	delete[] distanceTravelled;
	delete[] previousIndex;
	std::cout << "\nNo path possible!-----------------------------------------------------\n\n\n";
	return false;
}

float Algorithm::getHeuristic(int* currentGraph, int* xPositions, int* yPositions, int startIndex, int goalIndex) {
	float x1 = xPositions[currentGraph[startIndex]];
	float x2 = xPositions[currentGraph[goalIndex]];
	float y1 = yPositions[currentGraph[startIndex]];
	float y2 = yPositions[currentGraph[goalIndex]];
	float heuristics = (float)sqrt(abs(x2 - x1) * abs(x2 - x1) + abs(y2 - y1) * abs(y2 - y1));
	return heuristics;
}
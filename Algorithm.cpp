#include "BinaryHeap.hpp"
#include <cmath>
#include <chrono>
#include "Algorithm.hpp"
#include <limits>
#include "Renderer.hpp"
int Algorithm::currentIteration = -1;
bool Algorithm::findPath(int** o_pathXs, int** o_pathYs, int* o_pathLenght, Graph* graph, int startY, int startX, int goalY, int goalX) {
	currentIteration++;
	
	int startIndex = graph->getIndexFromCoords(startY, startX, true);
	int goalIndex = graph->getIndexFromCoords(goalY, goalX, true);

	int graphNodeCount = graph->getGraphNodeCount();

	graph->resetHeapIndices();

	//initialize GraphnodeHeuristics
	for (int nodeIndex = 0; nodeIndex < graphNodeCount; nodeIndex++) {
		for (int i = 0; i < graph->getIndexNeighbourCount()[nodeIndex]; i++) {
			int currentNeighbourIndex = graph->getNeighbourIndices()[nodeIndex][i];
			float heuristics = getHeuristic(graph->getGraph(), graph->getIndexBoundYs(), graph->getIndexBoundXs(), currentNeighbourIndex, goalIndex);
			graph->setNeighbourCost(nodeIndex, i, heuristics);
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
	HeapNode* toInsert = new HeapNode(getHeuristic(graph->getGraph(), graph->getIndexBoundYs(), graph->getIndexBoundXs(),
		graph->getGraph()[startIndex], graph->getGraph()[goalIndex]), startIndex);
	heap->insert(toInsert);
	bool foundPath = false;

	while (heap->getCurrentNodeCount() > 0) {//while heap is not empty
		HeapNode* helpNode = heap->extractMin();//extract best node
		int cNodeIndex = graph->getGraph()[helpNode->getIndexInGraph()];//get graphIndex of best node
		if (cNodeIndex == goalIndex) {
			foundPath = true;
			break;
		}
			int neighbourCount = graph->getIndexNeighbourCount()[cNodeIndex];//we will look through graph->getNeighbourIndices() of this node
			for (int i = 0; i < neighbourCount; i++) {
				int cNeighbourIndex = graph->getNeighbourIndices()[cNodeIndex][i];
				if (graph->isUsedByMoveableObject()[cNeighbourIndex] == false) {//efficient method to exclude moveable colision objects from graph
					float tempDistance = distanceTravelled[cNodeIndex] + graph->getIndexNeighbourCosts()[cNodeIndex][i];

					if (tempDistance < distanceTravelled[cNeighbourIndex]) {
						distanceTravelled[cNeighbourIndex] = tempDistance;
						previousIndex[cNeighbourIndex] = cNodeIndex;

						float heuristicOfCurrentNeighbour = tempDistance + getHeuristic(graph->getGraph(), graph->getIndexBoundYs(), 
							graph->getIndexBoundXs(), cNeighbourIndex, goalIndex);

						bool alreadyInserted = true;
						//if graphnode has been inserted to heap (index in heap initialized to -1)
						if (graph->getHeapIndices()[cNeighbourIndex] == -1) {
							alreadyInserted = false;
						}

						if (alreadyInserted == true) {
							heap->decrease(graph->getHeapIndices()[cNeighbourIndex], heuristicOfCurrentNeighbour);
						}
						else {
							HeapNode* nodeToInsert = new HeapNode(heuristicOfCurrentNeighbour, cNeighbourIndex);
							heap->insert(nodeToInsert);
						}
					}
				}
			}
		
		delete helpNode;
	}

	if (foundPath == true) {

		//std::cout << "time elapsed sind the algorithm started: " << Utils::endTimerGetTime();

		//get lenght of path array
		int pathLenght = 0;//goal pos pushed back
		int currentIndex = goalIndex;
		while (true) {
			currentIndex = previousIndex[currentIndex];
			pathLenght++;
			if (currentIndex == startIndex) {
				break;
			}
		}

		*o_pathXs = new int[pathLenght];
		*o_pathYs = new int[pathLenght];
		*o_pathLenght = pathLenght;

		//put path indices into path array from end to front

		int indexInPath = pathLenght - 1;
		currentIndex = goalIndex;
		while (true) {
			(*o_pathXs)[indexInPath] = graph->getIndexBoundXs()[currentIndex];
			(*o_pathYs)[indexInPath] = graph->getIndexBoundYs()[currentIndex];
			currentIndex = previousIndex[currentIndex];
			indexInPath--;
			if (currentIndex == startIndex) {
				break;
			}
		}

		(*o_pathXs)[pathLenght - 1] = goalX;
		(*o_pathYs)[pathLenght - 1] = goalY;

		if (pathLenght == 0) {
			std::cout << "\nNo path possible!-----------------------------------------------------\n\n\n";
			delete heap;
			delete[] distanceTravelled;
			delete[] previousIndex;
			return false;
		}
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

float Algorithm::getHeuristic(int* currentGraph, int* ys, int* xs, int startIndex, int goalIndex) {
	float x1 = xs[currentGraph[startIndex]];
	float x2 = xs[currentGraph[goalIndex]];
	float y1 = ys[currentGraph[startIndex]];
	float y2 = ys[currentGraph[goalIndex]];
	float heuristics = (float)sqrt(abs(x2 - x1) * abs(x2 - x1) + abs(y2 - y1) * abs(y2 - y1));
	return heuristics;
}
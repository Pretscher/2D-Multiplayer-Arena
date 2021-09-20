#include "BinaryHeap.hpp"
#include <cmath>
#include <chrono>
#include "Algorithm.hpp"
#include <limits>
#include "Renderer.hpp"


bool Algorithm::findPath(shared_ptr<int[]>& o_pathYs, shared_ptr<int[]>& o_pathXs, int& o_pathLenght, int startY, int startX, int goalY, int goalX) {
	nextIteration();
	int startIndex = graph->getIndexFromCoords(startY, startX, true);
	int goalIndex = graph->getIndexFromCoords(goalY, goalX, true);

	int graphNodeCount = graph->getGraphNodeCount();
	BinaryHeap* heap = new BinaryHeap(graph, graphNodeCount, currentIteration);

	graph->resetHeapIndices();

	float* distanceTravelled = new float[graphNodeCount];
	int* previousIndex = new int[graphNodeCount];

	for (int i = 0; i < graphNodeCount; i++) {
		distanceTravelled[i] = numeric_limits<float>::max();//Theoretically infinity, not used nodes have no valuable info
	}

	distanceTravelled[startIndex] = 0.0f;
	previousIndex[startIndex] = startIndex;

	//insert start node with the value 0
	HeapNode* toInsert = new HeapNode(getHeuristic(startIndex, goalIndex), startIndex);
	heap->insert(toInsert);
	bool foundPath = false;

	while (heap->getCurrentNodeCount() > 0) {//while heap is not empty
		HeapNode* helpNode = heap->extractMin();//extract best node
		int cNodeIndex = helpNode->getIndexInGraph(currentIteration);//get graphIndex of best node
		if (cNodeIndex == goalIndex) {
			foundPath = true;
			break;
		}
		for (int i = 0; i < graph->getIndexNeighbourCount()[cNodeIndex]; i++) {
			int currentNeighbourIndex = graph->getNeighbourIndices()[cNodeIndex][i];
			float heuristics = getHeuristic(currentNeighbourIndex, cNodeIndex);
			graph->setNeighbourCost(cNodeIndex, i, heuristics);
		}

		//we will look through graph->getNeighbourIndices() of this node
		for (int i = 0; i < graph->getIndexNeighbourCount()[cNodeIndex]; i++) {
			int cNeighbourIndex = graph->getNeighbourIndices()[cNodeIndex][i];
			if (graph->isUsedByMoveableObject()[cNeighbourIndex] == false) {//efficient method to exclude moveable colision objects from graph
				
				float tempDistance = distanceTravelled[cNodeIndex] + graph->getIndexNeighbourCosts()[cNodeIndex][i];
				if (tempDistance < distanceTravelled[cNeighbourIndex]) {
					distanceTravelled[cNeighbourIndex] = tempDistance;
					previousIndex[cNeighbourIndex] = cNodeIndex;

					float heuristicOfCurrentNeighbour = tempDistance + getHeuristic(cNeighbourIndex, goalIndex);

					//if graphnode has been inserted to heap (index in heap initialized to -1)
					if (graph->getHeapIndices()[cNeighbourIndex] != -1) {
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

		//cout << "time elapsed sind the algorithm started: " << Utils::endTimerGetTime();

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

		o_pathXs = shared_ptr<int[]>(new int[pathLenght]);
		o_pathYs = shared_ptr<int[]>(new int[pathLenght]);
		o_pathLenght = pathLenght;

		//put path indices into path array from end to front

		int indexInPath = pathLenght - 1;
		currentIndex = goalIndex;
		while (true) {
			o_pathXs[indexInPath] = graph->getIndexBoundXs()[currentIndex];
			o_pathYs[indexInPath] = graph->getIndexBoundYs()[currentIndex];
			currentIndex = previousIndex[currentIndex];
			indexInPath--;
			if (currentIndex == startIndex) {
				break;
			}
		}

		o_pathXs[pathLenght - 1] = goalX;
		o_pathYs[pathLenght - 1] = goalY;

		if (pathLenght == 0) {
			cout << "\nNo path possible!-----------------------------------------------------\n\n\n";
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
	cout << "\nNo path possible!-----------------------------------------------------\n\n\n";
	return false;
}

float Algorithm::getHeuristic(int startIndex, int goalIndex) {

	float x1 = graphXs[startIndex];
	float x2 = graphXs[goalIndex];
	float y1 = graphYs[startIndex];
	float y2 = graphYs[goalIndex];
	float heuristics = (float)sqrt(abs(x2 - x1) * abs(x2 - x1) + abs(y2 - y1) * abs(y2 - y1));
	return heuristics;
}
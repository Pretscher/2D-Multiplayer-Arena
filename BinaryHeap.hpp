#pragma once
#include "HeapNode.hpp"
#include "GraphNode.hpp"
#include "Graph.hpp"
class BinaryHeap {
public:
	BinaryHeap(Graph* graphNodes, unsigned int graphNodeCount);
	int getCurrentNodeCount();
	void actualizeGraphIndex(int indexInHeap);

	void insert(HeapNode* node);
	HeapNode* extractMin();
	void decrease(int heapIndex, float newKey);

	unsigned int getLeftChildIndex(int indexOfNodeInHeap);
	int getRightChildIndex(int indexOfNodeInHeap);
	int getParentIndex(int indexOfNodeInHeap);

	bool isRoot(int indexOfNodeInHeap);
	bool isLeaf(int indexOfNodeInHeap);

	void bubbleUp(int indexOfNodeInHeap);

	void bubbleDown(int indexOfNodeInHeap);
	~BinaryHeap() {
		for (int i = 0; i < nodeCount; i++) {
			delete[] heap[i];
		}
		delete[] heap;
	}
private:
	HeapNode** heap;
	Graph* graph;
	unsigned int nodeCount;
};

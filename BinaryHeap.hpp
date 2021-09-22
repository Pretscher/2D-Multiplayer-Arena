#pragma once
#include "HeapNode.hpp"
#include "Graph.hpp"
#include <memory>
class BinaryHeap {
public:
	//needs current iteration for efficiency: heap nodes are not deleted, but if they are reinserted into the heap and they were
	//initialized in another run of the algorithm, they reinitialize themselves.
	BinaryHeap(shared_ptr<Graph> graphNodes, unsigned int graphNodeCount, int currentIteration);
	int getCurrentNodeCount();
	void actualizeGraphIndex(int indexInHeap);

	void insert(shared_ptr<HeapNode> node);
	HeapNode extractMin();
	void decrease(int heapIndex, float newKey);

	unsigned int getLeftChildIndex(int indexOfNodeInHeap);
	int getRightChildIndex(int indexOfNodeInHeap);
	int getParentIndex(int indexOfNodeInHeap);

	bool isRoot(int indexOfNodeInHeap);
	bool isLeaf(int indexOfNodeInHeap);

	void bubbleUp(int indexOfNodeInHeap);

	void bubbleDown(int indexOfNodeInHeap);

private:
	vector<shared_ptr<HeapNode>> heap;
	shared_ptr<Graph> graph;
	unsigned int nodeCount;
	int currentIteration;
};

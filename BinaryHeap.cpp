#include "HeapNode.hpp"
#include "BinaryHeap.hpp"


	BinaryHeap::BinaryHeap(shared_ptr<Graph> graphNodes, unsigned int graphNodeCount, int currentIteration) {
		//heap has same size as graph
		nodeCount = 0;
		graph = graphNodes;
		heap = new HeapNode*[graphNodeCount];
		this->currentIteration = currentIteration;
	}

	int BinaryHeap::getCurrentNodeCount() {
		return nodeCount;
	}

	void BinaryHeap::actualizeGraphIndex(int indexInHeap) {
		unsigned int graphIndex = heap[indexInHeap]->getIndexInGraph(currentIteration);
		graph->getHeapIndices()[graphIndex] = indexInHeap;
	}

	//-------------------------------------------
	//Heap methods

	void BinaryHeap::insert(HeapNode* node) {
		int newIndex = nodeCount;
		nodeCount++;
		heap[newIndex] = node;
		actualizeGraphIndex(newIndex);
		bubbleUp(newIndex);
	}

	HeapNode* BinaryHeap::extractMin() {

		HeapNode* tempRoot = heap[0];
		nodeCount--;
		//overwrite with last leaf of heap
		heap[0] = heap[nodeCount];

		actualizeGraphIndex(0);
		bubbleDown(0);
		return tempRoot;
	}

	void BinaryHeap::decrease(int heapIndex, float newKey) {
		if (heap[heapIndex] != nullptr) {
			heap[heapIndex]->setKey(newKey);
			bubbleUp(heapIndex);
		}
	}

	//-------------------------------------------
	//Help heap methods

	//in these methods we do not check for array bounds for the sake of performance
	unsigned int BinaryHeap::getLeftChildIndex(int indexOfNodeInHeap) {
		int childIndex = 2 * indexOfNodeInHeap + 1;
		return childIndex;
	}

	int BinaryHeap::getRightChildIndex(int indexOfNodeInHeap) {
		int childIndex = 2 * (indexOfNodeInHeap + 1);
		return childIndex;
	}

	int BinaryHeap::getParentIndex(int indexOfNodeInHeap) {
		//if rightchild: automatically floors to same number
		int parentIndex = (indexOfNodeInHeap - 1) / 2;
		return parentIndex;
	}

	bool BinaryHeap::isRoot(int indexOfNodeInHeap) {
		return (indexOfNodeInHeap == 0);
	}

	bool BinaryHeap::isLeaf(int indexOfNodeInHeap) {
		if (indexOfNodeInHeap + 1 > nodeCount / 2) {
			return true;
		}
		else {
			return false;
		}
	}

	void BinaryHeap::bubbleUp(int indexOfNodeInHeap) {
		if (isRoot(indexOfNodeInHeap) == true) {
			return;
		}
		int tempParentIndex = getParentIndex(indexOfNodeInHeap);
		HeapNode* parent = heap[tempParentIndex];
		int tempIndex = indexOfNodeInHeap;
		HeapNode* tempNode = heap[indexOfNodeInHeap];

		while (isRoot(tempIndex) == false && parent->getKey() > tempNode->getKey()) {
			//swap node and parent
			heap[tempParentIndex] = tempNode;
			actualizeGraphIndex(tempParentIndex);
			heap[tempIndex] = parent;
			actualizeGraphIndex(tempIndex);
			//Set index of node to new place
			tempIndex = tempParentIndex;
			//get new parent index + HeapNode for the while condition
			tempParentIndex = getParentIndex(tempIndex);
			parent = heap[tempParentIndex];
		}
	}

	void BinaryHeap::bubbleDown(int indexOfNodeInHeap) {
		int tempChildIndex;
		HeapNode* tempChild;

		int tempIndex = indexOfNodeInHeap;
		HeapNode* tempNode = heap[indexOfNodeInHeap];

		while (isLeaf(tempIndex) == false) {

			//select child to swap with
			tempChildIndex = getLeftChildIndex(tempIndex);

			int rightChildIndex = getRightChildIndex(tempIndex);
			//if leftKey > rightKey swap
			//there does not need to be a right child for every non-leaf => check if there is
			if (rightChildIndex < nodeCount) {
				if ((*heap[tempChildIndex]).getKey() > (*heap[rightChildIndex]).getKey()) {
					tempChildIndex = rightChildIndex;
				}
			}
			//tempChildIndex can be the left or the right Childindex now
			tempChild = heap[tempChildIndex];
			//if heap has the right structure end this method
			if ((*tempChild).getKey() >= (*tempNode).getKey()) {
				break;
			}
			//swap node and parent
			heap[tempChildIndex] = tempNode;
			actualizeGraphIndex(tempChildIndex);
			heap[tempIndex] = tempChild;
			actualizeGraphIndex(tempIndex);
			//Set index of node to new place
			tempIndex = tempChildIndex;
		}
	}

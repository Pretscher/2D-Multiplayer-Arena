#pragma once
#include "Graph.hpp"
#include <mutex>
class Algorithm {
public:
	Algorithm(shared_ptr<Graph> i_graph, shared_ptr<std::mutex> io_mutex) {
		this->graph = i_graph;
		this->graphXs = graph->getIndexBoundXs();
		this->graphYs = graph->getIndexBoundYs();
		this->mutex = io_mutex;
		this->currentIteration = -1;
	}
	
	inline int getIteration() {
		this->mutex->lock();
		int iteration = this->currentIteration;
		this->mutex->unlock();
		return iteration;
	}

	inline void nextIteration() {
		this->mutex->lock();
		this->currentIteration ++;
		this->mutex->unlock();
	}

	bool findPath(shared_ptr<int[]>& o_pathYs, shared_ptr<int[]>& o_pathXs, int& o_pathLenght, int startY, int startX, int goalY, int goalX);
private:

	int currentIteration;//counts up with every pathfindings so that from outside the thread you can see if a pathfinding was completed
	shared_ptr<int[]> graphYs;
	shared_ptr<int[]> graphXs;
	shared_ptr<std::mutex> mutex;
	shared_ptr<Graph> graph;
	inline float getHeuristic(int startIndex, int goalIndex);
};
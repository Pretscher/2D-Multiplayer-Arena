#include "TimeThread.hpp"

MainThreadClock::MainThreadClock() {
	startTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	timeSinceStart = 0;
}

inline void MainThreadClock::update() {
	timeSinceStart = std::chrono::high_resolution_clock::now().time_since_epoch().count() - startTime;
}

inline long MainThreadClock::getTimeSinceStart() {
	return timeSinceStart;
}
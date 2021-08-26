#include <iostream>
#include <chrono>

class MainThreadClock {
public:
	MainThreadClock();
	inline void update();
	inline long getTimeSinceStart();
private:
	long startTime;
	long timeSinceStart;
};
#include <string>
#include "timeCondition.hpp"
#include <chrono>


TimeCondition::TimeCondition(double minute) {
	totalTimeInSec = minute * 60;
}
TimeCondition::TimeCondition(int hour, int minute, int second) {
	totalTimeInSec = hour * 3600 + minute * 60 + second;
}

void TimeCondition::start() {
	startClock = std::chrono::high_resolution_clock::now();
}
int TimeCondition::getPassedTime() {
	std::chrono::time_point<std::chrono::high_resolution_clock> endClock =
		std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::seconds>(endClock - startClock).count();
}
bool TimeCondition::chceckEndCondition() {
	return getPassedTime() >= totalTimeInSec;
}

double TimeCondition::percentComplet() {
	return (double)getPassedTime() / totalTimeInSec*100;
}


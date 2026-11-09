#pragma once
#include <string>
#include "endCondition.hpp"
#include <chrono>
#include <cmath>


class TimeCondition : public EndCondition {
public:
	TimeCondition(double minute);

	TimeCondition(int hour, int minute, int second);
	void start() override;
	bool chceckEndCondition() override; 
	double percentComplet() override;
	int getPassedTime();

private:
	int totalTimeInSec;
	std::chrono::time_point<std::chrono::high_resolution_clock> startClock;

};

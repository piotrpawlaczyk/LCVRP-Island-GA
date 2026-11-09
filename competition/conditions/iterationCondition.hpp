#pragma once
#include <string>
#include "endCondition.hpp"

class IterationCondition : public EndCondition{
public:
	IterationCondition(int numberIterations);

	virtual void start() override;
	virtual bool chceckEndCondition() override;
	virtual double percentComplet() override;


private:
	int numberLoop;
	int totalLoop;
};
#include "iterationCondition.hpp"


IterationCondition::IterationCondition(int numberIterations): totalLoop(numberIterations) {};
void IterationCondition::start() {
	numberLoop = 0;
}
bool IterationCondition::chceckEndCondition() {
	numberLoop++;
	return numberLoop >= totalLoop;
}


double IterationCondition::percentComplet() {
	return (double) numberLoop / totalLoop*100;
}


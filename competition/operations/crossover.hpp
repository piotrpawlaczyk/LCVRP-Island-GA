#pragma once
#include <vector>
#include <random>

#include "../population/indyvidual.hpp"
#include "../utility/fitnessEvaluator.hpp"
#include <algorithm>
#include <utility>
using namespace std;

class Crossover {
public:
	Crossover(FitnessEvaluator &fe, RND& rnd, double isCroosover) : fe(fe), rnd(rnd),
		isRuinCrossover(isCroosover), baseIsCrossoverRate(isCroosover),
		isCroosover(isCroosover){
		maxRate = min(baseIsCrossoverRate * 1.5, 0.9);
	}
	virtual ~Crossover() = default;
	virtual Indyvidual* createIndyvidual(Indyvidual* ind1, Indyvidual* ind2) = 0;

	virtual pair<Indyvidual*, Indyvidual*> croosover(Indyvidual* ind1, Indyvidual* ind2) {
		if (isRuinCrossover(rnd.rng() ))
			return { createIndyvidual(ind1, ind2), createIndyvidual(ind2, ind1) };
		return { ind1->clone(), ind2->clone()};
	}
	virtual void randomize() {

	}
	virtual void increaseDiversity() {

	}

protected:
	FitnessEvaluator& fe;
	RND& rnd;
	std::bernoulli_distribution isRuinCrossover;
	double baseIsCrossoverRate;
	double isCroosover;
	double maxRate;
};
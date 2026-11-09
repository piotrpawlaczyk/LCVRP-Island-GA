#pragma once

#include "../population/population.hpp"
#include <algorithm>
#include <cmath>

class Mutation {
public:
	Mutation(RND& rnd) : rnd(rnd) {}
	//virtual void makePreMuation(Population& pop) = 0;
	virtual void makeMuation(Indyvidual* ind) {
		
	
	};
	virtual void tryEscapeFromLocalMinimum(Population& pop) {
		Indyvidual* best = pop.getBest()->clone();
		for (Indyvidual* ind : pop.getAllIndyviduals()) {
			if (ind->getFitness() == best->getFitness() || rnd.randomBool(0.2)) {
				if (rnd.randomBool(0.5))
					ind->insertMutation(0.1, 0.4);
				else if (rnd.randomBool(0.5)) {
					for (int i = 0; i < (int)max(2.0, sqrt(rnd.getRandomGroup() / 3)); i++)
						ind->randomRoutes();
				}
				else	ind->swapMutation(0.1, 0.2, 0.4);
			}
		}
		pop.add(best);
		pop.finished();

	};
	virtual void extremeEscape(Population& pop) {
		Indyvidual* best = pop.getBest()->clone();
		for (Indyvidual* ind : pop.getAllIndyviduals()) {
			if (rnd.randomBool(0.2))
				ind->insertMutation(0.5, 0.6);
			else if (rnd.randomBool(0.8))
				ind->swapMutation(0.4, 0.8, 0.6);
			else {
				ind->insertMutation(0.5, 0.6);
				ind->swapMutation(0.4, 0.8, 0.6);
			}


		}
		pop.add(best);
		pop.finished();

	};


	virtual ~Mutation() = default;

public:
	RND& rnd;
};

#include "../templete/Evaluator.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include "tournamentSelection.hpp"
#include "../population/population.hpp"
using namespace std;

/*
TournamentSelection::TournamentSelection(FitnessEvaluator& e, int size_tournament,
	double probabilityOfWorser): Selection(e),
	probability(uniform_real_distribution<double>(0.0, 1.0)),
	getIndex(uniform_int_distribution<int>(0, 0)){
	population = nullptr;
};
*/

TournamentSelection::TournamentSelection( int sizeTournament, RND& rnd): rnd(rnd),
	sizeTournament(sizeTournament),getIndex(uniform_int_distribution<int>(0, 0)) {
	population = nullptr;
};

void TournamentSelection::initializeSelection(Population* pop) {
	population = pop;
	getIndex = uniform_int_distribution<int>(0, population->size()-1);
}

Indyvidual* TournamentSelection::getIndyvidual() {
	int index = getIndex(rnd.rng());
	for (int i = 1; i < sizeTournament; i++) {
		int nextIndex = getIndex(rnd.rng());
		//	nextIndex = nextIndex;
		if (index > nextIndex)
			index = nextIndex;
	}
	return population->get(index);
}

#pragma once

#include <vector>
#include <random>
#include "../utility/fitnessEvaluator.hpp"
#include "../population/indyvidual.hpp"
#include "selection.hpp"


class TournamentSelection : public Selection {
public:
	//TournamentSelection(FitnessEvaluator& e, int size_tournament,double probabilityOfWorser);
	TournamentSelection(int size_tournament, RND& rnd);

	virtual void initializeSelection(Population* pop) override;
	virtual Indyvidual* getIndyvidual() override;

	void setSize(int size) {
		sizeTournament = size;
	}
	virtual void bigSeletion() override {
		sizeTournament = 10;
	}

private:
	uniform_int_distribution<int> getIndex;
	RND& rnd;
	int sizeTournament;
	double probabilityOfWorser;
};
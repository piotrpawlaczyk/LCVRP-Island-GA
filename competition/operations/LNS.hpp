#pragma once

#include "mutation.hpp"


class LNS : public Mutation {
public:
	LNS(double _baseRatioOfLS, bool _isOrderLS, bool _isReversLS, 
		bool _isGreedyOrderLS, bool _isGreedyReversLS, RND& rnd, double probabilityWorser = 0);
	~LNS() override = default;

	//void makePreMuation(Population& pop) override;
	void makeMuation(Indyvidual* ind) override;

private:

	bool isOrderLS;
	bool isReversLS ;
	bool isGreedyOrderLS;
	bool isGreedyReversLS;
	double baseRatioOfLS = 0;
	double probabilityWorser;
	//double preEvolutionRatioOfLS = 0;
};
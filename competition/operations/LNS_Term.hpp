#pragma once

#include "mutation.hpp"


class LNS_Term : public Mutation {
public:
	LNS_Term(int term, double  ratioPart, double ratioFull, RND& rnd, int& i);
	~LNS_Term() override = default;

	void makeMuation(Indyvidual* ind) override;

private:
	int term;
	int& i;
	double  ratioPart;
	double ratioFull;
	//double preEvolutionRatioOfLS = 0;
};
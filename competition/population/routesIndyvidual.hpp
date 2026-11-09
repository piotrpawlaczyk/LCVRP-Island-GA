#pragma once
#include <iostream>
#include <vector>
#include "population.hpp"
#include "indyvidual.hpp"
#include "routes.hpp"
#include "../utility/rnd.hpp"
#include <algorithm>
#include <queue>
#include <list>
#include <stack>
#include <set>
class RoutesIndyvidual : public Indyvidual {
public:
	RoutesIndyvidual(FitnessEvaluator& fe, RND& rnd, vector<Routes> groups,
		double fitness);
	RoutesIndyvidual(FitnessEvaluator& fe, RND& rnd, vector<Routes> groups);

	RoutesIndyvidual(RoutesIndyvidual& other);
	virtual Indyvidual* clone() override;
	~RoutesIndyvidual() override = default;

	virtual void escapeMuation(int numberGroupFrom1, int numberGroupFrom2,
		int numberGroupTo1, int numberGroupTo2) override;
	virtual void escapeMuation(int numberGroupFrom1, int numberGroupFrom2) override;
	virtual bool swapMutation(double probabilitySwapRoutes, double probabilitySwap, double probabilityToAcceptWorser) override;
	virtual bool insertMutation(double probablityInsert, double probabilityToAcceptWorser) override;
	virtual bool insertLocalSearch(double probablityInsert) override;
	virtual bool LNSlocalSearch(bool isOrderLS, bool isReversLS, bool isGreedyOrderLS, bool isGreedyReversL, 
		double probabilityWorser) override;
	virtual void randomRoutes(double probabilityBetter = 0.6) override;

	bool seachWithGrouping(int index1, int index2);
	virtual vector<int> toNumberedGroup() override;

	vector<Routes> getGroup() { //UWAGA KOPIA
		return groups;
	}
	virtual void print() override;

private:
	vector<Routes> groups;
};

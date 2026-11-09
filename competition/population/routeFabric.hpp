#pragma once
#include "../utility/fitnessEvaluator.hpp"
#include <iostream>
#include <vector>
#include "population.hpp"
#include "indyvidual.hpp"
#include "routes.hpp"
#include "routesIndyvidual.hpp"
#include <list>

using namespace std;

class RouteFabric {
public:
	RouteFabric(FitnessEvaluator& fe, RND& rnd) :fe(fe), rnd(rnd) {}

	vector<Indyvidual*> initGiantInidviduals(int size);
	vector<Indyvidual*> initInidviduals(double q = 2);
	vector<Indyvidual*> initInidvidualsV2(double q = 2);
	vector<Indyvidual*> initReinforceIndyviduals(double q, Indyvidual* last);

	vector<Indyvidual*> creator(int selectedGroupSize, int bigTournamntSize, int selectedGroupSizeReverse,
		int randomSize, bool nearestGroup, vector<Indyvidual*> pop = {});


	Indyvidual* initToGroup(int(*func)(FitnessEvaluator&, vector<Routes>&, int));
	Indyvidual* initBestToNearestGroup();
	Indyvidual* initToNearestBigTournamntGroup();
	Indyvidual* initToNearestSelectedGroup();
	Indyvidual* initToNearestGroup();
	Indyvidual* initRandomGroup();
	Indyvidual* initSelectedGroupReverse();

	int addToSelectedGroup(vector<Routes>& groups,
		int indexToAdd, int minGroup = 0, bool isMinCostarints = true,
		bool isMoreRandom = false);
	//int addToSelectedGroupReverse(FitnessEvaluator& fe, vector<Routes>& groups,
	//	int indexToAdd);

	int addToGroup(vector<Routes>& groups, int indexToAdd, int sizeOfTournamnt = -1);
	int addBestToGroup(vector<Routes>& groups, int indexToAdd);
	int addToRandomGroup(vector<Routes>& groups, int indexToAdd);

private:
	RND& rnd;
	FitnessEvaluator& fe;
};

#include <iostream>
#include "routeFabric.hpp"
#include "routes.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <vector>

vector<Indyvidual*> RouteFabric::initGiantInidviduals(int size) {
	vector<Indyvidual*> pop;
	int selectedGroupSize = 0.6 * size;
	int selectedGroupSizeReverse = 0 * size;
	bool nearestGroup = true;
	int randomSize = size - selectedGroupSize - nearestGroup;
	assert(randomSize >= 0);

	return creator(selectedGroupSize, 0, 0, randomSize, nearestGroup);
}
vector<Indyvidual*> RouteFabric::initInidviduals(double q) {
	vector<Indyvidual*> pop;
	int size = (double)(q * fe.getNumCustomers());
	int selectedGroupSize = 0.36 * size;
	int bigTournamntSize = 0.12 * size;
	int selectedGroupSizeReverse = 0.3 * size;
	bool nearestGroup = true;
	int randomSize = size - selectedGroupSize - bigTournamntSize - nearestGroup;
	assert(randomSize >= 0);

	return creator(selectedGroupSize, bigTournamntSize, selectedGroupSizeReverse, randomSize, nearestGroup);
}

vector<Indyvidual*> RouteFabric::initInidvidualsV2(double q) {
	int size = (double)(q * fe.getNumCustomers());
	int selectedGroupSize = 0.3 * size;
	int bigTournamntSize = 0.2 * size;
	int selectedGroupSizeReverse = 0.2 * size;
	bool nearestGroup = true;
	int randomSize = size - selectedGroupSize - bigTournamntSize - nearestGroup;
	assert(randomSize >= 0);

	return creator(selectedGroupSize, bigTournamntSize, selectedGroupSizeReverse, randomSize, nearestGroup);
}


vector<Indyvidual*> RouteFabric::initReinforceIndyviduals(double q, Indyvidual* last) {
	int size = (double)(q * fe.getNumCustomers());
	int selectedGroupSize = 0.2 * size;
	int bigTournamntSize = 0.05 * size;
	int selectedGroupSizeReverse = 0.1 * size;
	bool nearestGroup = true;
	int randomSize = size - selectedGroupSize - bigTournamntSize - nearestGroup;
	assert(randomSize >= 0);
	vector<Indyvidual*> pop;
	pop.push_back(last);
	return creator(selectedGroupSize, bigTournamntSize, selectedGroupSizeReverse, randomSize, nearestGroup,
		pop);
}



vector<Indyvidual*> RouteFabric::creator(int selectedGroupSize, int bigTournamntSize, int selectedGroupSizeReverse,
	int randomSize, bool nearestGroup, vector<Indyvidual*> pop) {

	if (nearestGroup)
		pop.push_back(initToNearestGroup());
	for (int i = 0; i < bigTournamntSize; i++) {
		pop.push_back(initToNearestBigTournamntGroup());
	}
	for (int i = 0; i < selectedGroupSize; i++) {
		pop.push_back(initToNearestSelectedGroup());
	}
	for (int i = 0; i < selectedGroupSizeReverse; i++) {
		pop.push_back(initSelectedGroupReverse());
	}
	for (int i = 0; i < randomSize; i++) {
		pop.push_back(initRandomGroup());
	}
	return pop;
}

int RouteFabric::addToGroup(vector<Routes>& groups, int indexToAdd, int sizeOfTournamnt) {

	if (sizeOfTournamnt == -1) sizeOfTournamnt = max(20.0, sqrt(fe.getNumGroups()));

	//za malo preferuje przejscie przez depota
	int groupIndex = rnd.getRandomGroup();
	for (int i = 1; i < sizeOfTournamnt; i++) {
		int nextIndex = rnd.getRandomGroup();
		if (groups[groupIndex].getDistanceToIndex(indexToAdd) >
			groups[nextIndex].getDistanceToIndex(indexToAdd))
			groupIndex = nextIndex;
	}
	groups[groupIndex].addIndex(indexToAdd);

	return groupIndex;
}
int RouteFabric::addBestToGroup(vector<Routes>& groups, int indexToAdd) {
	//za malo preferuje przejscie przez depota
	int groupIndex = 0;
	for (int i = 1; i < fe.getNumGroups(); i++) {
		if (groups[groupIndex].getDistanceToIndex(indexToAdd) >
			groups[i].getDistanceToIndex(indexToAdd))
			groupIndex = i;
	}
	groups[groupIndex].addIndex(indexToAdd);

	return groupIndex;
}
int RouteFabric::addToRandomGroup(vector<Routes>& groups, int indexToAdd) {
	int groupIndex = rnd.getRandomGroup();
	groups[groupIndex].addIndex(indexToAdd);
	return groupIndex;
}


int RouteFabric::addToSelectedGroup(vector<Routes>& groups,
	int indexToAdd, int minGroup, bool isMinCostarints, bool isMoreRandom) {
	vector<int> groupsIndexsInOrder;

	for (int i = minGroup; i < fe.getNumGroups(); i++) {
		if (!isMinCostarints || indexToAdd > groups[i].getLast())
			groupsIndexsInOrder.push_back(i);
	}
	/*
	//stare ustawienia: 
	int numberPotentialGroup = groupsIndexsInOrder.size();
	int potantialChoose = 1 + max(1.0, sqrt(numberPotentialGroup));
	int sizeOfTournamnt = max(2.0, sqrt(numberPotentialGroup));
	assert(numberPotentialGroup >= 2);
	*/
	int numberPotentialGroup = groupsIndexsInOrder.size();
	int potantialChoose = numberPotentialGroup;
	int sizeOfTournamnt;
	if (isMoreRandom) sizeOfTournamnt = 2;
	else sizeOfTournamnt = max(2.0, sqrt(numberPotentialGroup));
	
	sort(groupsIndexsInOrder.begin(), groupsIndexsInOrder.end(), [&groups, indexToAdd](int a, int b) {
		return 	groups[a].getDistanceToIndex(indexToAdd) < groups[b].getDistanceToIndex(indexToAdd);
		});

	std::uniform_int_distribution<int> indexGroupGen{ 0, potantialChoose - 1 };
	int groupIndex = groupsIndexsInOrder[indexGroupGen(rnd.rng())];
	for (int i = 1; i < sizeOfTournamnt; i++) {
		int nextIndex = groupsIndexsInOrder[indexGroupGen(rnd.rng())];;
		if (groups[groupIndex].getDistanceToIndex(indexToAdd) >
			groups[nextIndex].getDistanceToIndex(indexToAdd))
			groupIndex = nextIndex;
	}
	groups[groupIndex].addIndex(indexToAdd);

	return groupIndex;
}


Indyvidual* RouteFabric::initBestToNearestGroup() {

	vector<Routes> groups;

	for (int i = 0; i < fe.getNumGroups(); i++) 
		groups.emplace_back(fe, rnd);
	

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addBestToGroup(groups, i);

	for (Routes& group : groups) {
		group.closeRoute();
	}

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);
	return ind;
}

Indyvidual* RouteFabric::initToGroup(int(*addFunction)(FitnessEvaluator&, vector<Routes>&, int)) {
	vector<Routes> groups;
	
	for (int i = 0; i < fe.getNumGroups(); i++)
		groups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addFunction(fe, groups, i);
	for (Routes& group : groups)
		group.closeRoute();

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);

	return ind;
}

Indyvidual* RouteFabric::initToNearestSelectedGroup() {
	vector<Routes> groups;
	
	for (int i = 0; i < fe.getNumGroups(); i++)
		groups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addToSelectedGroup(groups, i);
	//addToGroup(fe, groups, i);
	for (Routes& group : groups)
		group.closeRoute();

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);

	return ind;
}

Indyvidual* RouteFabric::initSelectedGroupReverse() {
	vector<Routes> reverseGroups;
	vector<Routes> finalGroups;

	for (int i = 0; i < fe.getNumGroups(); i++)
		reverseGroups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumGroups(); i++)
		finalGroups.emplace_back(fe, rnd);


	for (int i = fe.getNumCustomers()-1; i >= 0; i--)
		// addToSelectedGroup(fe, reverseGroups, i, 0, false);
		addToGroup(reverseGroups, i);

	for (Routes& group : reverseGroups) {
		group.closeRoute();
	}

	for (int numberGroup = 0; numberGroup < fe.getNumGroups(); numberGroup++) {
		vector<int> indexes = reverseGroups[numberGroup].toVector();
		reverse(indexes.begin(), indexes.end());

		for (int index: indexes)
			finalGroups[numberGroup].addIndex(index);
		finalGroups[numberGroup].closeRoute();
	}

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, finalGroups);
	return ind;
}

Indyvidual* RouteFabric::initToNearestBigTournamntGroup() {
	vector<Routes> groups;

	for (int i = 0; i < fe.getNumGroups(); i++)
		groups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addToGroup(groups, i);
	for (Routes& group : groups)
		group.closeRoute();

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);

	
	return ind;
}
Indyvidual* RouteFabric::initToNearestGroup() {
	vector<Routes> groups;

	for (int i = 0; i < fe.getNumGroups(); i++)
		groups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addBestToGroup(groups, i);
	for (Routes& group : groups)
		group.closeRoute();

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);
	return ind;
}
Indyvidual* RouteFabric::initRandomGroup() {
	vector<Routes> groups;
	
	for (int i = 0; i < fe.getNumGroups(); i++)
		groups.emplace_back(fe, rnd);

	for (int i = 0; i < fe.getNumCustomers(); i++)
		addToRandomGroup(groups, i);
	for (Routes& group : groups)
		group.closeRoute();

	Indyvidual* ind = new RoutesIndyvidual(fe, rnd, groups);
	return ind;
}




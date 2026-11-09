#include <iostream>
#include <vector>
#include "routesIndyvidual.hpp"
#include <algorithm>
#include <cmath>
#include <cassert>
#include <set>
#include <cstddef>


using namespace std;



RoutesIndyvidual::RoutesIndyvidual(FitnessEvaluator& fe, RND& rnd, vector<Routes> groups,
	double fitness) : Indyvidual(fe, rnd, fitness), groups(groups) {
	for (int i = 0; i < fe.getNumGroups(); i++)
		groups[i].closeRoute();
}

RoutesIndyvidual::RoutesIndyvidual(FitnessEvaluator& fe, RND& rnd, vector<Routes> groups) :
	Indyvidual(fe, rnd), groups(groups) {
	for (int i = 0; i < fe.getNumGroups(); i++)
		groups[i].closeRoute();
	fitness = 0;
	for (Routes route : groups) {
		fitness += route.getFitness();
	}
}
RoutesIndyvidual::RoutesIndyvidual(RoutesIndyvidual& other): Indyvidual(other) {
	groups = other.groups;
};
Indyvidual* RoutesIndyvidual::clone() {
	return new RoutesIndyvidual(*this);
}


bool RoutesIndyvidual::seachWithGrouping(int index1, int index2) {
	return false;
}

void RoutesIndyvidual::escapeMuation(int numberGroupFrom1, int numberGroupFrom2,
	int numberGroupTo1, int numberGroupTo2) {
	double delta = 0;
	delta += groups[numberGroupFrom1].randomise(groups[numberGroupFrom2]);
	delta += groups[numberGroupFrom1].LNSWholeGroups(groups[numberGroupTo1]);
	delta += groups[numberGroupFrom2].LNSWholeGroups(groups[numberGroupTo2]);
	fitness -= delta;

}
void RoutesIndyvidual::escapeMuation(int numberGroupFrom1, int numberGroupFrom2) {
	double totalDelta = 0;
	totalDelta += groups[numberGroupFrom1].randomise(groups[numberGroupFrom2]);

	set<int> toCheck;
	toCheck.insert(numberGroupFrom1);
	toCheck.insert(numberGroupFrom2);
	while (!toCheck.empty()) {
		int indexGroup = *toCheck.begin();
		toCheck.erase(toCheck.begin());
		int otherIndex = rnd.randomGroupIndexOtherThan(indexGroup);

		double delta = groups[indexGroup].LNSWholeGroups(groups[otherIndex]);
		if (delta > 0.01) {
			toCheck.insert(otherIndex);
		}
		totalDelta += delta;
	}
	fitness -= totalDelta;
}


bool RoutesIndyvidual::swapMutation(double probabilitySwapRoutes, double probabilitySwap,
	double probabilityToAcceptWorser) {
	for (int i = 0; i < fe.getNumGroups(); i++) {
		int j;
		do {
			j = rnd.getRandomGroup();
		} while (j == i);
		if (rnd.randomBool(probabilitySwapRoutes)) {
			double delta = groups[i].swaping(groups[j], probabilitySwap, probabilityToAcceptWorser);
			fitness -= delta;

		}
		
	}
	return false;
}

bool RoutesIndyvidual::insertMutation(double probablityInsert, double probabilityToAcceptWorser ) {
	for (int i = 0; i < fe.getNumGroups(); i++) {
		int j = rnd.randomGroupIndexOtherThan(i);
		double delta = groups[i].inserting(groups[j], probablityInsert, probabilityToAcceptWorser);
		fitness -= delta;	
	}
	return false;
}

bool  RoutesIndyvidual::insertLocalSearch(double probablityInsert) {
	set<int> toCheck;
	for (int i = 0; i < fe.getNumGroups(); i++) {
		toCheck.insert(i);
	}
	bool isImprove = false;
	while (!toCheck.empty()) {
		int i = *toCheck.begin();
		toCheck.erase(toCheck.begin());
		for (int j = 0; j < fe.getNumGroups(); j++) {
			if (i != j) {
				double delta = groups[i].inserting(groups[j], 0, 1);
				fitness -= delta;
				if (delta > 0.01) {
					toCheck.insert(i);
					toCheck.insert(j);
					isImprove = true;
				}
			}
		}
	}
	return isImprove;
	


}

bool RoutesIndyvidual::LNSlocalSearch(bool isOrderLS, bool isReversLS, bool isGreedyOrderLS,
	bool isGreedyReversLS, double probabilityWorser) {
	//ociupinke przyspiesza (strzelam ze najwalniejszy i tak jest kross)
	//seachWithGrouping(); //uwaga on sie NIE NADAJE dla ograniczenia dystansu
	//if (isFull)
	//	isReverse = true;
	if (isLocalMin)
		return false;
	isLocalMin = true;
	set<int> toCheck;



	double delta;
	vector<int> firsts( fe.getNumGroups() );
	for (int i = 0; i < fe.getNumGroups(); i++)
		firsts[i] = i;
	vector<int> seconds = firsts;
	shuffle(firsts.begin(), firsts.end(), rnd.rng());
	shuffle(seconds.begin(), seconds.end(), rnd.rng());

	if (isOrderLS) {
		for (int i:firsts) {
			for (int j: seconds) {
				if (i == j) continue;
				double delta = groups[i].LNSWholeGroups(groups[j], 
					probabilityWorser);;
				if (delta) {
					//przelicz ponownie totalFitness
					toCheck.insert(i);
					toCheck.insert(j);
					fitness -= delta;
				}
			}
		}
	}

	if (isReversLS) {
		for (int i : firsts) {
			for (int j : seconds) {
				if (i == j) continue;
				double delta = groups[i].LNSWholeGroupsReverse(groups[j], probabilityWorser);
				if (delta) {
					//przelicz ponownie totalFitness
					toCheck.insert(i);
					toCheck.insert(j);
					fitness -= delta;
				}
			}
		}
	}
	if ( (!isGreedyOrderLS  && !isGreedyReversLS ) || toCheck.empty())
		return toCheck.size();

	while (!toCheck.empty()) {
		int i = *toCheck.begin();
		toCheck.erase(toCheck.begin());

		bool isImprove = false;
		for (int j : seconds) {
			if (i != j) {
				if (isGreedyOrderLS) {
					delta = groups[i].LNSWholeGroups(groups[j], probabilityWorser);;
					if (delta != 0.0) {
						toCheck.insert(i);
						toCheck.insert(j);
						fitness -= delta;
						isImprove = true;
					}
					else isImprove = false;
				}
				if(isGreedyReversLS && (!isImprove)) {
					delta = groups[i].LNSWholeGroupsReverse(groups[j], probabilityWorser);
					if (delta != 0.0) {
						toCheck.insert(i);
						toCheck.insert(j);
						fitness -= delta;
					}
				}				
			}
		}
	}
	return true;


}
void RoutesIndyvidual::randomRoutes(double probabilityBetter) {
	if (groups.size() > 2) {
		int index1 = rnd.getRandomGroup();
		int index2;
		int index3;
		do 	index2 = rnd.getRandomGroup();
		while (index1 == index2);
		do 	index3 = rnd.getRandomGroup();
		while (index1 == index3 || index2 == index3);

		double oldFitnessThisGroups = groups[index1].getFitness() + groups[index2].getFitness() + groups[index3].getFitness();
		double newFitnessThisGroups = groups[index1].randomise(groups[index2],
			groups[index3], probabilityBetter);
		fitness = fitness + newFitnessThisGroups - oldFitnessThisGroups;
	}


}

vector<int> RoutesIndyvidual::toNumberedGroup() {
	const vector<int> mapIndexToValue = fe.getIndexToCustomer();

	vector<int> result;
	result.resize(mapIndexToValue.size());
	for (size_t i = 0; i < groups.size(); i++)	{
		Routes routes = groups[i];
		for(int indexCustomer: routes.toVector())
			result[mapIndexToValue[indexCustomer] - 1] = i;
	}

	return result;
}

void RoutesIndyvidual::print() {
	for (Routes& routes : groups) {
		cout << "Group: ";
		cout << routes.toString();
		cout << '\n';
	}
	cout << "Indyvidual fitness " << fitness << "\n";

}
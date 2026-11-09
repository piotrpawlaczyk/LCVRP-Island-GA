#include <iostream>
#include <vector>
#include "routes.hpp"
#include <algorithm>
#include <cassert>
#include <queue>
#include <map>
#include <random>
#include <string>
#include <utility>

Routes& Routes::operator=(Routes other) {
	swap(routes, other.routes);
	swap(totalFitness, other.totalFitness);
	swap(isClosed, other.isClosed);
	swap(thisRoute, other.thisRoute);

	return *this;
};

Routes& Routes::operator=(Routes&& other) {
	if (this == &other)
		return *this;

	routes = move(other.routes);
	totalFitness = move(other.totalFitness);
	isClosed = move(other.isClosed);
	thisRoute = move(other.thisRoute);

	return *this;
};


void Routes::addIndex(int index) {
	if (!isAddPossible(index))
		endRoute();
	addIndexToRoute(index);
	isClosed = false;
}

void Routes::addIndexToRoute(int index) {
	thisRoute.fitness += fe.getIndexDistanceWithDepot(thisRoute.last(), index);
	thisRoute.indexes.push_back(index);
	thisRoute.capacity += fe.getDemandsIndex(index);
}

bool Routes::isAddPossible(int index) {
	return thisRoute.capacity + fe.getDemandsIndex(index) <= fe.getMaxCapacity() &&
		(!fe.hasDistanceConstraint() || thisRoute.size() == 0 ||
			thisRoute.fitness + fe.getIndexDistance(thisRoute.last(), index) 
			+ fe.getEnd(index) <= fe.getMaxDistance());
}

void Routes::activeRoute() {
	if (isClosed && !routes.empty()) {
		thisRoute = routes.back();
		routes.pop_back();

		totalFitness -= thisRoute.fitness;
		thisRoute.fitness -= fe.getEnd(thisRoute.last());
		isClosed = false;
	}
}


void Routes::closeRoute() {
	if(!isClosed) endRoute();
	isClosed = true;
}

void Routes::endRoute() {
	thisRoute.fitness += fe.getEnd(thisRoute.last());
	totalFitness += thisRoute.fitness;

	routes.push_back(thisRoute);
	thisRoute = { {}, 0, 0 };
}

double Routes::getStrightDistance(int index) {
		return fe.getIndexDistanceWithDepot(thisRoute.last(), index);
}

double Routes::getDistanceToIndex(int index) {
	if (isAddPossible(index))
		return fe.getIndexDistanceWithDepot(thisRoute.last(), index);
	else 
		return fe.getEnd(thisRoute.last())+fe.getStart(index);
}

//to moze byc po porsu wykonanie mutacji, bo o takiej mutacji myslalem
double Routes::insertCity(Routes& other) {
	return 0.0;
}



double Routes::swaping(Routes& other, double probablityInsert, double probabilityToAcceptWorser) {

	if (routes.size() != 1 || other.routes.size() != 1)
		return 0;

	Route& routeFromSwap = routes[0];
	Route& routeToSwap = other.routes[0];

	int placeFromSwap = 0;
	int placeToSwap = 0;
	double totalDeltaFitness = 0;

	while (placeFromSwap < routeFromSwap.size() && placeToSwap < routeToSwap.size() ) {

		int index = routeFromSwap.get(placeFromSwap);
		int prevIndex = getPreviousIndex(0, placeFromSwap);
		int nextIndex = getNextIndex(0, placeFromSwap);

		int index2 = routeToSwap.get(placeToSwap);
		int prevIndex2 = other.getPreviousIndex(0, placeToSwap);
		int nextIndex2 = other.getNextIndex(0, placeToSwap);

		if (index2 > nextIndex || index < prevIndex2) {
			placeFromSwap++;
			continue;
		}
		if (index2 < prevIndex || index > nextIndex2) {
			placeToSwap++;
			continue;
		}


		double newFirstDistance = routeFromSwap.fitness -
			fe.getIndexDistanceWithDepot(prevIndex, index) - fe.getIndexDistanceWithDepot(index, nextIndex) +
			fe.getIndexDistanceWithDepot(prevIndex, index2) + fe.getIndexDistanceWithDepot(index2, nextIndex);

		double newSecondDistance = routeToSwap.fitness -
			fe.getIndexDistanceWithDepot(prevIndex2, index2) - fe.getIndexDistanceWithDepot(index2, nextIndex2) +
			fe.getIndexDistanceWithDepot(prevIndex2, index) + fe.getIndexDistanceWithDepot(index, nextIndex2);


		double deltaFitness = routeFromSwap.fitness + routeToSwap.fitness - newFirstDistance - newSecondDistance;
		int newFirstCapacity = routeFromSwap.capacity + fe.getDemandsIndex(index2) - fe.getDemandsIndex(index);
		int newSecondCapacity = routeToSwap.capacity + fe.getDemandsIndex(index) - fe.getDemandsIndex(index2);
		if ((newFirstCapacity < fe.getMaxCapacity() && newSecondCapacity < fe.getMaxCapacity()) &&
			(deltaFitness > 0 || rnd.randomBool(probabilityToAcceptWorser)) &&
			(!fe.hasDistanceConstraint() || (newFirstDistance <= fe.getMaxDistance() &&
				newSecondDistance <= fe.getMaxDistance()))) {


			routeFromSwap.indexes[placeFromSwap] = index2;
			routeToSwap.indexes[placeToSwap] = index;


			routeFromSwap.capacity = newFirstCapacity;
			routeToSwap.capacity = newSecondCapacity;

			totalDeltaFitness += deltaFitness;

			totalFitness += newFirstDistance - routeFromSwap.fitness;
			other.totalFitness += newSecondDistance - routeToSwap.fitness;

			routeFromSwap.fitness = newFirstDistance;
			routeToSwap.fitness = newSecondDistance;
		}
		placeToSwap++;
	}
	return totalDeltaFitness;
}


//mega kosztowne
double Routes::inserting(Routes& other, double probablityInsert, double probabilityToAcceptWorser) {
	//TO VECTOR RAZ TWOPRZYC I POZNIEJ PTZEKAZYWAC KPIE (TWORZY W CLOSE) PRZECIEZ WASKIE GARDLO!!!!


	//pozniej to dopuscic oraz MNIEJ RND BOOLA - waskie gardlo!!!!!!!!!!!!!!!!!!
	//kolejnosc niestety moze zaklamywac to...
	if ( routes.size() != 1 || other.routes.size() != 1 || totalFitness == 0 || other.totalFitness == 0)
		return 0;

	Route& thisRoute = routes[0];
	Route& otherRoute = other.routes[0];


	int placeToInsert = 0;
	double totalDeltaFitness = 0;
	for (int i = 0; i < thisRoute.size() && thisRoute.size() > 1; i++) {
		if (rnd.randomBool(probablityInsert)) {
			int indexToInsert = thisRoute.get(i);
			//czy dziala NIE DZIALAAAAAAAAAAAAAAA!!!!!!!!!!!!?
			while (otherRoute.get(placeToInsert) < indexToInsert)
				placeToInsert++;
			if (otherRoute.capacity + fe.getDemandsIndex(indexToInsert) <= fe.getMaxCapacity()) {

				double newFirstDistance = thisRoute.fitness -  fe.getIndexDistanceWithDepot(thisRoute.get(i - 1), indexToInsert) - fe.getIndexDistanceWithDepot(indexToInsert, thisRoute.get(i + 1)) +
					 + fe.getIndexDistanceWithDepot(thisRoute.get(i - 1), thisRoute.get(i + 1));
				
				double newSecondDistance = otherRoute.fitness - fe.getIndexDistanceWithDepot(otherRoute.get(placeToInsert - 1), otherRoute.get(placeToInsert))
					+ fe.getIndexDistanceWithDepot(otherRoute.get(placeToInsert - 1), indexToInsert) + fe.getIndexDistanceWithDepot(indexToInsert, otherRoute.get(placeToInsert));;
				
				double deltaFitness = thisRoute.fitness + otherRoute.fitness - newFirstDistance - newSecondDistance;
					
				if ((deltaFitness > 0 || rnd.randomBool(probabilityToAcceptWorser)) &&
					(!fe.hasDistanceConstraint() || (newFirstDistance <= fe.getMaxDistance() &&
						newSecondDistance <= fe.getMaxDistance()) )) {
					thisRoute.indexes.erase(thisRoute.indexes.begin() + i);
					otherRoute.indexes.insert(otherRoute.indexes.begin() + placeToInsert, indexToInsert);
					i--;
					totalDeltaFitness += deltaFitness;
					thisRoute.capacity -= fe.getDemandsIndex(indexToInsert);
					otherRoute.capacity += fe.getDemandsIndex(indexToInsert);
					thisRoute.fitness = newFirstDistance;
					otherRoute.fitness = newSecondDistance;
					totalFitness = newFirstDistance;
					other.totalFitness = newSecondDistance;

				}
			}

		}
	}
	if (thisRoute.size() == 0)
		routes.clear();

	return totalDeltaFitness;
}

double Routes::LNSWholeGroups(Routes& other, double probabilityWorser) {
	vector<int> indexes = toVector();
	vector<int> indexesFrom2 = other.toVector();

	//vector<int> indexes;
	//indexes.reserve(indexesFrom1.size() + indexesFrom2.size());
	int size = indexes.size();
	indexes.reserve(indexes.size() + indexesFrom2.size());
	indexes.insert(indexes.end(), indexesFrom2.begin(), indexesFrom2.end());
	inplace_merge(indexes.begin(), indexes.begin() + size, indexes.end());

	Routes routes1{ fe , rnd };
	Routes routes2{fe, rnd };
	//bernoulli_distribution dist(1- probabilityWorser);

	if(probabilityWorser > 0.00001){
		bernoulli_distribution moreFalse(probabilityWorser);
		for (int index : indexes) {
			if ((routes1.getDistanceToIndex(index) < routes2.getDistanceToIndex(index)) ^ moreFalse(rnd.rng()))
				//&& dist(rnd.rng())
				routes1.addIndex(index);
			else
				routes2.addIndex(index);
		}
	}
	else 
		for (int index : indexes) {
		if (routes1.getDistanceToIndex(index) < routes2.getDistanceToIndex(index)
		//&& dist(rnd.rng())
			)
			routes1.addIndex(index);
		else
			routes2.addIndex(index);
	}

	routes1.closeRoute();
	routes2.closeRoute();

	//zrobic move semantic!!!!!!!!!!!!!!!!!!!!
	if (totalFitness + other.totalFitness > routes1.totalFitness + routes2.totalFitness) {
		double fitnessDelta = totalFitness + other.totalFitness - (routes1.totalFitness + routes2.totalFitness);
		
		swap(routes, routes1.routes);
		swap(other.routes, routes2.routes);
		swap(totalFitness, routes1.totalFitness);
		swap(other.totalFitness, routes2.totalFitness);

		return fitnessDelta;
	}
	return 0;
}

double Routes::randomise(Routes& other, bool isFull) {
	vector<int> indexes = toVector();
	vector<int> indexesFrom2 = other.toVector();


	int sizeIndex = indexes.size();
	double probToCut = min(1.0, 0.01 / ( (0.001 + sizeIndex + indexesFrom2.size()) / fe.getNumCustomers()));

	//jak zadziala bez map
	map<int, int> indexesToRoute;
	for (int i : indexes)
		indexesToRoute[i] = 1;

	for (int i : indexesFrom2)
		indexesToRoute[i] = 2;

	indexes.reserve(indexes.size() + indexesFrom2.size());
	indexes.insert(indexes.end(), indexesFrom2.begin(), indexesFrom2.end());
	inplace_merge(indexes.begin(), indexes.begin() + sizeIndex, indexes.end());


	double oldFitnessesSum = totalFitness + other.totalFitness;
	reset();
	other.reset();

	bernoulli_distribution dist(probToCut);
	for (int index : indexes) {
		if ( (indexesToRoute[index] == 1) ^ dist(rnd.rng()) )
			addIndex(index);
		else
			other.addIndex(index);
	}

	closeRoute();
	other.closeRoute();

	return oldFitnessesSum - totalFitness - other.totalFitness;

}
double Routes::randomise(Routes& other1, Routes& other2, double probabilityBetter) {
	vector<int> indexes = toVector();
	vector<int> indexesFrom1 = other1.toVector();
	vector<int> indexesFrom2 = other2.toVector();

	indexes.insert(indexes.end(), indexesFrom1.begin(), indexesFrom1.end());
	indexes.insert(indexes.end(), indexesFrom2.begin(), indexesFrom2.end());
	sort(indexes.begin(), indexes.end());

	reset();
	other1.reset();
	other2.reset();
	int distanceToThisRoute;
	int distanceToRoute1;
	int distanceToRoute2;
	uniform_real_distribution<double> dist(0.0, 1.0);
	vector < pair<double, Routes*>> pairs;

	Routes* other1Pointer = &other1;
	Routes* other2Pointer = &other2;

	double firstRankVal = 0.5;
	double secondRankVal = 0.35;
	for (int index : indexes) {
		double dist0 = 1.0 / max(0.1,getDistanceToIndex(index));
		double dist1 = 1.0 / max(0.1,other1.getDistanceToIndex(index));
		double dist2 = 1.0 / max(0.1,other2.getDistanceToIndex(index));
		double sumDist = dist0 + dist1 + dist2;
		pairs = {
			{dist0/ sumDist, this},
			{dist1 / sumDist, other1Pointer},
			{dist2 / sumDist, other2Pointer}
		};
		sort(pairs.begin(), pairs.end(), greater<>());	//brak co dla rownych fitnesow
		double randVal = dist(rnd.rng());
		if (randVal < (pairs[0].first + firstRankVal) / 2)
			pairs[0].second->addIndex(index);

		else if ( (randVal - (pairs[0].first + firstRankVal) / 2 ) <
			(pairs[1].first + firstRankVal) / 2)
			pairs[1].second->addIndex(index);

		else 
			pairs[2].second->addIndex(index);
	}
	closeRoute();
	other1.closeRoute();
	other2.closeRoute();
	return getFitness() + other1.getFitness() + other2.getFitness();
};


double Routes::LNSWholeGroupsReverse(Routes& other, double probabilityWorser) {
	if (routes.size() != 1 || other.routes.size() != 1)
		return 0;

	vector<int> indexes = toVector();
	vector<int> indexesFrom2 = other.toVector();
	indexes.insert(indexes.end(), indexesFrom2.begin(), indexesFrom2.end());

	//da sie szybciej ale zmezony jestem
	sort(indexes.begin(), indexes.end(), greater<>());
	//reverse(indexes.begin(), indexes.end());


	Routes RevRoutes1{ fe, rnd };			//routes1
	Routes RevRoutes2{ fe, rnd };

	Routes routes1{ fe, rnd };			//routes1
	Routes routes2{ fe, rnd };
	//patrzec na capacity

	if (probabilityWorser > 0.00001) {
		bernoulli_distribution moreFalse(probabilityWorser);

		for (int index : indexes) {
			if ((RevRoutes1.getDistanceToIndex(index) < RevRoutes2.getDistanceToIndex(index)) ^ moreFalse(rnd.rng())
				)
				RevRoutes1.addIndex(index);
			else
				RevRoutes2.addIndex(index);
		}
	}
	else {
		for (int index : indexes) {
			if (RevRoutes1.getDistanceToIndex(index) < RevRoutes2.getDistanceToIndex(index))
				RevRoutes1.addIndex(index);
			else
				RevRoutes2.addIndex(index);
		}
	}

	RevRoutes1.closeRoute();
	RevRoutes2.closeRoute();

	//znowu mozna zoptymalizowac, ale to pozmnmiej
	vector<int> indexes1 = RevRoutes1.toVector();
	vector<int> indexes2 = RevRoutes2.toVector();
	reverse(indexes1.begin(), indexes1.end());
	reverse(indexes2.begin(), indexes2.end());


	for (int ind : indexes1)
		routes1.addIndex(ind);
	for (int ind : indexes2)
		routes2.addIndex(ind);

	routes1.closeRoute();
	routes2.closeRoute();

	if (totalFitness + other.totalFitness > routes1.totalFitness + routes2.totalFitness) {
		double fitnessDelta = totalFitness + other.totalFitness - (routes1.totalFitness + routes2.totalFitness);

		swap(routes, routes1.routes);
		swap(other.routes, routes2.routes);
		swap(totalFitness, routes1.totalFitness);
		swap(other.totalFitness, routes2.totalFitness);

		return fitnessDelta;
	}
	return 0;
}




Routes::Routes(const Routes& other): fe(other.fe), rnd(other.rnd) {
	routes = other.routes;
	totalFitness = other.totalFitness;
	isClosed = other.isClosed;
	thisRoute = other.thisRoute;
}

string Routes::toString() {
	string s = "";
	for (Route& route:routes) {
		s += route.toString() + "   ";
	}
	if (!isClosed) {
		s += "aktualny " + thisRoute.toString();
		s += "total " + to_string(totalFitness);
	}
	return s;
}

//uwaga bez aktualnego

vector<int> Routes::toVector(){
	vector<int> indexes;

	for(Route& route: routes)
		indexes.insert(indexes.end(), route.indexes.begin(), route.indexes.end());

	return indexes;

}
vector<int> Routes::toVectorWithEnds() {
	vector<int> indexes;
	assert(isClosed);
	indexes.push_back(-1);
	for (Route& route : routes)
		indexes.insert(indexes.end(), route.indexes.begin(), route.indexes.end());
	indexes.push_back(fe.getNumCustomers());
	return indexes;

}



#pragma once
#include <vector>
#include "../utility/fitnessEvaluator.hpp"
#include "../utility/rnd.hpp"
#include <limits>
#include <algorithm>
#include <string>


struct Route {
	vector<int> indexes;
	double fitness;
	double capacity ;
	int depot = -1;
	int endDepot = std::numeric_limits<int>::max();

	//Route() {};
	//Route(vector<int> indexes, double fitness, double capacity) :
	//	indexes(indexes), fitness(fitness), capacity(capacity) {};
	int last() const { if (indexes.empty()) return depot; else return indexes.back(); }
	int get(int i) const {
		if (i == -1) return depot;
		if (i >= indexes.size()) return endDepot;
		return indexes[i]; 
	}
	int size() const { return indexes.size(); }

	string toString() const {
		string s = "";
		for (int i : indexes) s += to_string(i) + " ";
		s += "f: " + to_string(fitness);
		s += " c: " + to_string(capacity);
		return s;
	}

	string toStringCust(FitnessEvaluator &fe) const {
		string s = "";
		for (int i : indexes) s += to_string( fe.getIndexToCustomer()[i]) + " ";
		s += "f: " + to_string(fitness);
		s += " c: " + to_string(capacity);
		return s;
	}
	
};

class Routes {
public:
	Routes(FitnessEvaluator& fe, RND& rnd) : fe(fe), rnd(rnd) {
		thisRoute = { {}, 0, 0 };
	};

	Routes(const Routes& other);

	Routes& operator=(Routes other);
	Routes& operator=(Routes&& other);

	void reset() {
		routes.clear();
		totalFitness = 0;
		isClosed = true;
		thisRoute = { {}, 0, 0 };
	}
	double getFitness() { return totalFitness; }

	
	void closeRoute();

	void activeRoute();

	void addIndex(int index);
	void addIndexToRoute(int inde);

	bool isAddPossible(int index);

	//vector<Route> getThisRoute();

	//Route getRoute();

	//void addRoute(Route route);
	double getStrightDistance(int index);
	double getDistanceToIndex(int index);

	bool ended() {
		return isClosed;
	}
	double getCapacityCurrentRoute() { return thisRoute.capacity; }
	string toString();
	vector<int> toVector();
	vector<int> toVectorWithEnds();

	
	double swaping(Routes& other, double probablityInsert = -1, double probabilityToAcceptWorser = 0);
	double inserting(Routes& other, double probablityInsert = -1, double probabilityToAcceptWorser = 0);
	double LNSWholeGroups(Routes& other, double probabilityWorser = 0);
	double LNSWholeGroupsReverse(Routes& other, double probabilityWorser = 0);
	double randomise(Routes& other1,  Routes& other2, double probabilityBetter);
	double randomise(Routes& other1, bool isFull=false);


	int getLast() {
		if (isClosed)  return depot; else return thisRoute.last();
	}
	double insertCity(Routes& other); 

	vector <int> getFirstCities() {
		vector<int> firtCities;
		for (Route& route : routes)
			firtCities.push_back(route.indexes[0]);
		return firtCities;
	}


	int getPreviousIndex(int routeNumber, int index_) {
		if (index_ == 0) {
			if (routeNumber == 0)
				return depot;
			else return routes[routeNumber - 1].last();
		}
		return routes[routeNumber].get(index_ - 1);
	}
	int getNextIndex(int routeNumber, int index_) {
		if (routes[routeNumber].size() == 1+ index_) {
			if (routeNumber + 1 == routes.size() )
				return depot;
			else return routes[routeNumber + 1].get(0);
		}
		return routes[routeNumber].get(index_ + 1);
	}

private:
	void endRoute();
	FitnessEvaluator& fe;
	RND& rnd;

	std::vector<Route> routes;
	double totalFitness = 0;
	bool isClosed = true;

	Route thisRoute;
	const int depot = -1;
};
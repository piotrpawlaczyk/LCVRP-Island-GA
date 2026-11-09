#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include "indyvidual.hpp"
#include "../utility/rnd.hpp"
#include <map>
#include <functional>
#include <iostream>
#include <random>

class Population {
public:
	//jakos inicjalizacja
	Population(vector<Indyvidual*> indyviduals, RND &rnd): indyviduals(indyviduals), rnd(rnd) { 
		couterLocal = 0;
		finished();
	};
	Population(RND& rnd) : rnd(rnd) {};

	Population(Population &other):rnd(other.rnd) {
		couterLocal = other.couterLocal;
		isFinished = other.isFinished;
		eliteSize = other.eliteSize;

		for (Indyvidual* ind : other.indyviduals)
			indyviduals.push_back(ind->clone());

		baseRatioOfLS = other.baseRatioOfLS;
		preEvolutionRatioOfLS = other.preEvolutionRatioOfLS;
		isOrderLS = other.isOrderLS;
		isReversLS = other.isReversLS;
		isGreedyOrderLS = other.isGreedyOrderLS;
		isGreedyReversLS = other.isGreedyReversLS;
	};

	~Population() {
		for (Indyvidual* ind : indyviduals)
			delete ind;
	}

	void setLocalSearchRatios(double _preEvolutionRatioOfLS, double _baseRatioOfLS,
		bool _isOrderLS, bool _isReversLS,
		bool _isGreedyOrderLS, bool _isGreedyReversLS) {
		baseRatioOfLS = _baseRatioOfLS;
		preEvolutionRatioOfLS = _preEvolutionRatioOfLS;
		isOrderLS = _isOrderLS;
		isReversLS = _isReversLS;
		isGreedyOrderLS = _isGreedyOrderLS;
		isGreedyReversLS = _isGreedyReversLS;
	}
	void setLocalSearchRatios(Population& other){
		baseRatioOfLS = other.baseRatioOfLS;
		preEvolutionRatioOfLS = other.preEvolutionRatioOfLS;
		isOrderLS = other.isOrderLS;
		isReversLS = other.isReversLS;
		isGreedyOrderLS = other.isGreedyOrderLS;
		isGreedyReversLS = other.isGreedyReversLS;
	}



	Population& operator=(const Population& other) {
		if (this == &other)
			return *this;
		couterLocal = other.couterLocal;
		isFinished = other.isFinished;
		eliteSize = other.eliteSize;
		for (Indyvidual* ind : indyviduals)
			delete ind;
		indyviduals.clear();
		for (Indyvidual* ind : other.indyviduals)
			indyviduals.push_back(ind->clone());

		return *this;
	}
	

	//ewemntualnie zachowac rozne osobniki
	void addElite(Population& other, double coefficientOfElite = 0.01 ) {
		int size = max(1.0, coefficientOfElite * (other.size()) );
		size += size % 2;
		//int maxNum = 10 * coefficientOfElite * (other.size();
		for (int i = 0; i < size; i++) {
			indyviduals.push_back(other.indyviduals[i]->clone());
		}
	}
	void finished() {
		if (!isFinished) {
			std::sort(indyviduals.begin(), indyviduals.end(), [](Indyvidual* ind1, Indyvidual* ind2) {
				return ind1->getFitness() < ind2->getFitness();
				});

			isFinished = true;
		}
	};
	void add(Indyvidual* ind) { 
		indyviduals.push_back(ind);
		isFinished = false;
	};
	double getDiversity() {
		double div = 0;
		for (Indyvidual* ind1 : indyviduals)
			for (Indyvidual* ind2 : indyviduals)
				div += ind1->getDiversity(ind2);
		return div;
	};

	void makePreLocalSearch() {
		for (Indyvidual* ind : indyviduals)
			if (rnd.randomBool(preEvolutionRatioOfLS)) {
				ind->LNSlocalSearch(isOrderLS, isReversLS, isGreedyOrderLS, isGreedyReversLS);
				couterLocal++;
			}
		finished();
	}

	void makeAllLocalSearch() {
		for(Indyvidual* ind: indyviduals)
			ind->LNSlocalSearch(isOrderLS, isReversLS, isGreedyOrderLS, isGreedyReversLS);
		finished();
	}

	void makeLocalSearch() {
		for (Indyvidual* ind : indyviduals)
			if (rnd.randomBool(baseRatioOfLS)) {
				ind->LNSlocalSearch(isOrderLS, isReversLS, isGreedyOrderLS, isGreedyReversLS);
				couterLocal++;
			}
		finished();
	}

	int size() { return indyviduals.size(); }

	Indyvidual* getMediana() { return indyviduals[size() / 2]; };
	Indyvidual* getBest() { return indyviduals[0]; };
	Indyvidual* get(int index) { return indyviduals[index]; }
	vector<Indyvidual*>& getInd() {
		return indyviduals;
	}

	void printBests(int number) {
		if (!isFinished) finished();
		for (int i = 0; i < number && i < size(); i++) {
			indyviduals[i]->print();
			cout << '\n';
		}
		cout << '\n';
	}
	void printRandomFitness(int number) {
		vector<double> fitnesses;
		int term = 14;
		if (number >= size()) 
			for (int i = 0; i < size(); i++)
				fitnesses.push_back(indyviduals[i]->getFitness());
		else {
			uniform_int_distribution<int> dist{ 0, size() - 1 };
			for (int i = 0; i < number; i++)
				fitnesses.push_back(indyviduals[dist(rnd.rng())]->getFitness());
		}
		sort(fitnesses.begin(), fitnesses.end());

		for (int i = 0; i < fitnesses.size();i++) {
			cout << fitnesses[i] << ' ';
			if (i % term == term - 1)
				cout << '\n';
		}
		cout << '\n';
	}

	void printRandomFitnessWithOg(int number) {
		vector<Indyvidual*> inds;
		if (number >= size())
			for (int i = 0; i < size(); i++)
				inds.push_back(indyviduals[i]);
		else {
			uniform_int_distribution<int> dist{ 0, size() - 1 };
			for (int i = 0; i < number; i++)
				inds.push_back(indyviduals[dist(rnd.rng())]);
		}
		std::sort(inds.begin(), inds.end(), [](Indyvidual* ind1, Indyvidual* ind2) {
			return ind1->getFitness() < ind2->getFitness();
			});


		for (int i = 0; i < inds.size(); i++) {
			cout << inds[i]->getFitness()  << '\n';
		}
		cout << '\n';
	}
	vector<Indyvidual*>& getAllIndyviduals() { return indyviduals; }
	int couterLocal = 0;

protected:
	RND& rnd;
	bool isFinished = false;
	int eliteSize = 10;
	vector<Indyvidual*> indyviduals;

	double baseRatioOfLS = 0;
	double preEvolutionRatioOfLS = 0;

	bool isOrderLS = false;
	bool isReversLS = false;
	bool isGreedyOrderLS = false;
	bool isGreedyReversLS = false;
};

/*
->push_back(childs.first->mutation());
		newPopulation->push_back(childs.second->mutation())
		
		*/
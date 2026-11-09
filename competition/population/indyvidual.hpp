#pragma once
#include <vector>
#include <utility>
#include <algorithm>
#include <limits>
#include <iomanip>
#include "../templete/ProblemData.hpp"
#include "../utility/fitnessEvaluator.hpp"
#include <iostream>

using namespace std;
class Indyvidual {
public:
	//Indyvidual(const ProblemData& data):data(data){
	//	fitness
	//};
	Indyvidual(FitnessEvaluator& fe, RND& rnd, double fitness): fe(fe), rnd(rnd), fitness(fitness){};
	Indyvidual(FitnessEvaluator& fe, RND& rnd) : fe(fe), rnd(rnd) {};
	Indyvidual(Indyvidual& other) : rnd(other.rnd), fe(other.fe) {
		fitness = other.fitness;
		isLocalMin = other.isLocalMin;
	};
	virtual ~Indyvidual() = default;
	virtual Indyvidual* clone() = 0;
	virtual vector<int> toNumberedGroup() = 0;
	virtual double getFitness() { return fitness; };
	virtual double getDiversity(Indyvidual*) { return 0; };
	bool operator<(const Indyvidual& other) const {
		return fitness < other.fitness;
	}
	virtual void print() = 0;
	virtual void escapeMuation(int numberGroupFrom1, int numberGroupFrom2,
		int numberGroupTo1, int numberGroupTo2) = 0;
	virtual void escapeMuation(int numberGroupFrom1, int numberGroupFrom2) = 0;
	virtual bool swapMutation(double probabilitySwapRoutes, double probabilitySwap, double probabilityToAcceptWorser) = 0;
	virtual bool insertMutation(double probablityInsert , double probabilityToAcceptWorser) = 0;
	virtual bool insertLocalSearch(double probablityInsert) = 0;
	virtual bool LNSlocalSearch(bool isOrderLS, bool isReversLS, bool isGreedyOrderLS,
		bool isGreedyReversL, double probabilityWorser = 0) = 0;
	virtual void randomRoutes(double probabilityBetter = 0.6) = 0;
	virtual double getOgFitness() {
		return fe.getEvaluator().Evaluate(toNumberedGroup());
	};
	void printNumberedGroup() {
		vector<int>  v = toNumberedGroup();
		for (int i : v)
			cout << i << ' ';
		cout << '\n';
	}
protected:
	double fitness;
	FitnessEvaluator& fe;
	RND& rnd;
	bool isLocalMin = false;
};
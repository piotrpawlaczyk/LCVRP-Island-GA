#include <vector>
#include <random>
#include <iostream>
#include <limits>
#include <iomanip>

#include <algorithm>
#include "../templete/Evaluator.hpp"
#include "GA_Optimalizator.hpp"
#include "../operations/tournamentSelection.hpp"
#include "../population/population.hpp"
#include "../utility/rnd.hpp"
#include "../population/routeFabric.hpp"
#include <cmath>
#include <utility>

using namespace std;

GA_Optimalizator::GA_Optimalizator(FitnessEvaluator& fe, RND* rnd, Population population,
	Selection* selection, Crossover* crossover, Mutation* mutation, 
	TimeCondition& endCondition): 
	fe(fe), rnd(rnd), population(population), selection(selection),
	crossover(crossover), mutation(mutation), properSize( population.size() ), 
	endCondition(endCondition) {}

GA_Optimalizator::GA_Optimalizator(FitnessEvaluator& fe, RND* rnd, Population population,
	Selection* selection, Crossover* crossover, Mutation* mutation,
	TimeCondition& endCondition, double minuteToReset) :
	fe(fe), rnd(rnd), population(population), selection(selection),
	crossover(crossover), mutation(mutation), properSize(population.size()),
	endCondition(endCondition) {
	resetTermLengh = minuteToReset / totalTime * 100;
}

void GA_Optimalizator::Initialize() {
	//pozniej tutajprzeniose najlepszy kontruktor
	current_best_fitness_ = population.getBest()->getFitness();
	current_mediana_fitness_ = population.getMediana()->getFitness();
	properSize = population.size();

	nextShakeTime = endCondition.percentComplet() + 2*shakeTermLength;
	nextResetTime = endCondition.percentComplet() + 2*resetTermLengh;

}

void GA_Optimalizator::RunIteration() {
		Population newPopulation(*rnd);
		newPopulation.setLocalSearchRatios(population);
		newPopulation.addElite(population, eliteRatio);
		selection->initializeSelection(&population);
		
		endIsolation();
		//rationNewSize
		int newSize = properSize;
		
		for (int i = newPopulation.size(); i + 2 <= newSize; i += 2) {
			pair<Indyvidual*, Indyvidual*> parents = selection->getTwoIndyviduals();

			pair<Indyvidual*, Indyvidual*> childs = crossover->croosover(parents.first, parents.second);

			mutation->makeMuation(childs.first);
			mutation->makeMuation(childs.second);

			newPopulation.add(childs.first);
			newPopulation.add(childs.second);
		}

		newPopulation.finished();

		int index = maxRatioOfBest * newPopulation.size();
		if (rnd->randomBool(0.1) &&
			newPopulation.get(index)->getFitness() == newPopulation.get(0)->getFitness())
			mutation->tryEscapeFromLocalMinimum(newPopulation);
		//	checkDiversity();
		//newPopulation.makeLocalSearch();

		shakePopulation();
		double oldBest = population.getBest()->getFitness();

		if (abs (population.getBest()->getFitness() - 
			newPopulation.getBest()->getFitness()) > 0.01) {
			nextShakeTime = endCondition.percentComplet() + shakeTermLength;
			nextResetTime = endCondition.percentComplet() + resetTermLengh;
		}
		//dodac MOVE
		population = move(newPopulation);

		current_best_fitness_ = population.getBest()->getFitness();
		current_mediana_fitness_ = population.getMediana()->getFitness();
}

void GA_Optimalizator::printAfterLoop(int i) {
	cout << "wykonano: " << endCondition.percentComplet() << "%, petla: " << i << ", najlepszy : " << population.getBest()->getFitness() << ' ';
	cout << population.getBest()->getOgFitness() << ' ';
	cout << " mediana: " << population.getMediana()->getFitness() << '\n';
	population.printRandomFitness(16);
	cout << '\n';
}


Indyvidual* GA_Optimalizator::getBest() {
	return population.getBest();
}


void GA_Optimalizator::worseMigration(vector<GA_Optimalizator*> otherIslands) {
	if ((lastWorseMigrationBestFitnes > current_best_fitness_ || lastWorseMigrationMedianaFitnes >
		current_mediana_fitness_) && !isIsolate) {
		for (GA_Optimalizator* island: otherIslands) {
			if (island == this) continue;
			if (current_best_fitness_ * 1.01< island->current_best_fitness_) {
				int number = max(ratioMigartionIfImprovment * island->getSize(), minMigartion);
				for (int i = 0; i < number; i++) addIndyvidual(island, 
					ratioOfInydvidualToMigrationFromWorserIslandUp, 
					ratioOfInydvidualToMigrationFromWorserIslandDown);
			}
			else {
				int number = max(ratioMigartionIfImprovment * island->getSize(), minMigartion);
				for (int i = 0; i < number; i++)
					addIndyvidual(island, ratioOfInydvidualToMigrationFromBetterIslandUp, 
						ratioOfInydvidualToMigrationFromBetterIslandDown, false);
			}
		}
	}
	else if(!isIsolate) {
		for (GA_Optimalizator* island : otherIslands) {
			if (island == this) continue;
			if (current_best_fitness_ * 1.01 < island->current_best_fitness_) {
				int number = max(higherRatioMigartionIfStagnation * island->getSize(), minMigartion);
				for (int i = 0; i < number; i++) addIndyvidual(island, 
					ratioOfInydvidualToMigrationFromWorserIslandUp, 
					ratioOfInydvidualToMigrationFromWorserIslandDown
					);
			}
			else {
				int number = max(lowerRatioMigartionIfStagnation * island->getSize(), minMigartion);
				for (int i = 0; i < number; i++)
					addIndyvidual(island, ratioOfInydvidualToMigrationFromBetterIslandUp,
						ratioOfInydvidualToMigrationFromBetterIslandDown, false);
			}
		}
	}
	lastWorseMigrationBestFitnes = current_best_fitness_;
	lastWorseMigrationMedianaFitnes = current_mediana_fitness_;
	population.finished();
}

void GA_Optimalizator::endIsolation() {
	if(startOfIsolate + isolateTime < endCondition.percentComplet())
		isIsolate = false;
}

void GA_Optimalizator::reset(vector<GA_Optimalizator*> otherIslands) {
	if (endCondition.percentComplet() > nextResetTime) {
		nextResetTime = endCondition.percentComplet() + resetTermLengh;
		if (isIsolate) isIsolate = false;
		else{
			double bestFitness = numeric_limits<double>::max();

			for (GA_Optimalizator* island : otherIslands) {
				if (island != this) {
					bestFitness = min(bestFitness, island->GetCurrentBestFitness());
				}
			}
			//nie current_best_fitness_ < bestFitness 
			if (current_best_fitness_ > bestFitness
				|| abs(current_best_fitness_ - bestFitness) < 0.01) { //the same
				startOfIsolate = endCondition.percentComplet();
				isIsolate = true;
				RouteFabric fabric(fe, *rnd);
				population = Population(fabric.initInidviduals(
					1.0 * population.size() / fe.getNumCustomers()), *rnd);
				Initialize();
			}
			else {
				mutation->extremeEscape(population);
			}


		}
		lastResetBestFitnes = current_best_fitness_;

	}
}

void GA_Optimalizator::shakePopulation() {
	if (nextShakeTime < endCondition.percentComplet()) {
		nextShakeTime += shakeTermLength;
		mutation->tryEscapeFromLocalMinimum(population);
	}
}
/*
void GA_Optimalizator::checkDiversity() {
	if (lastShakeBestFitnes <= current_best_fitness_) {
		mutation->tryEscapeFromLocalMinimum(population);
		crossover->increaseDiversity();
		lastShakeBestFitnes = current_best_fitness_;
		lastShakeMedianaFitnes = current_mediana_fitness_;
		cout << ":(";
	}
}
*/


void GA_Optimalizator::addIndyvidual(GA_Optimalizator* island, 
	double ratioOfInydvidualToMigrationUp, double ratioOfInydvidualToMigrationDown, 
	bool isCanBeBest) {
	int down = ratioOfInydvidualToMigrationDown * island->population.size();
	int up = ratioOfInydvidualToMigrationUp * island->population.size() - 1;
	if (down > up) swap(down, up);
	int index = rnd->rndTo(up - down)  + down;
	
	if (!isCanBeBest && abs(island->population.get(index)->getFitness() - island->GetCurrentBestFitness()) < 1.001) {
		down = min(0.8, 1.25 * ratioOfInydvidualToMigrationDown) * island->population.size();;
		up = min(1.0, 1.25 * ratioOfInydvidualToMigrationUp) * island->population.size() - 1;
		if (down > up) swap(down, up);
		index = rnd->rndTo(up - down) + down;
	}
	Indyvidual* copyIndyvidual = island->population.get(index)->clone();
	//cout << "MIGRACJA osobnika" << copyIndyvidual->getFitness() << ' ';

	population.add(copyIndyvidual);;
	//cout << population.get(0)->getFitness() << '\n';
}

void GA_Optimalizator::fenixRises() {
	shakeTermLength = 4.0 / totalTime * 100;
	ratioToCheckDiversity = 0.1;
	maxRatioOfBest = 0.4;
}

void GA_Optimalizator::endMigration(vector<GA_Optimalizator*> otherIslands) {
	for (GA_Optimalizator* island : otherIslands) {
		if (island != this) {
			int number = 0.1 * population.size();
			for (int i = 0; i < number; i++)
				addIndyvidual(island, ratioOfInydvidualToMigrationFromWorserIslandUp, 
					ratioOfInydvidualToMigrationFromWorserIslandDown);
		}
	}

	shakeTermLength = 0.1;
	nextShakeTime = endCondition.percentComplet();

	ratioToCheckDiversity = 0.9;
	maxRatioOfBest = 0.1;
	
	population.finished();
	mutation->tryEscapeFromLocalMinimum(population);
	population.finished();
	lastShakeBestFitnes = current_best_fitness_;
	lastShakeMedianaFitnes = current_mediana_fitness_;

	selection->bigSeletion();
	crossover->randomize();
}



/*virtual void Initialize() override;
		virtual void RunIteration() override;
		virtual vector<int>* GetCurrentBest() override;
		virtual vector<int>* GetCurrentMediana() override;
		virtual double getDiversity() override;*/

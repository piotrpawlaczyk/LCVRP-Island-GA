#pragma once

#include <vector>
#include <random>
#include "../templete/Evaluator.hpp"
#include "../population/population.hpp"
#include "../operations/selection.hpp"
#include "../operations/mutation.hpp"
#include "../operations/crossover.hpp"
#include "../conditions/timeCondition.hpp"

using namespace std;

class GA_Optimalizator{
public:
	GA_Optimalizator(FitnessEvaluator& fe, RND* rnd, Population population,
		Selection* selection, Crossover* crossover, Mutation* mutation, 
		TimeCondition& endCondition);

	GA_Optimalizator(FitnessEvaluator& fe, RND* rnd, Population population,
		Selection* selection, Crossover* crossover, Mutation* mutation,
		TimeCondition& endCondition, double minuteToReset);

	~GA_Optimalizator() {
		delete selection;
		delete crossover;
		delete mutation;
		delete rnd;
	}

	void Initialize();
	void RunIteration();
	Indyvidual* getBest();
	double GetCurrentBestFitness() { return current_best_fitness_; }

	void printAfterLoop(int i);

	void worseMigration(vector< GA_Optimalizator* > otherIslands);
	void reset(vector< GA_Optimalizator* > otherIslands);
	void endIsolation();
	void endMigration(vector<GA_Optimalizator*> otherIslands);		//migrantion when time is going to end
	void fenixRises();
	void shakePopulation();
	//void checkDiversity();
	void addIndyvidual(GA_Optimalizator* island,
		double ratioOfInydvidualToMigrationUp, double ratioOfInydvidualToMigrationDown = 0,
		bool isCanBuBest = true);

	int getSize() { return population.size(); };
	void setTerm(double shakeTermMinute, double resetTermMinute) {
		shakeTermLength = shakeTermMinute / totalTime * 100;
		resetTermLengh = resetTermMinute / totalTime * 100;
	}

	
private:
	FitnessEvaluator& fe;
	RND* rnd;
	//niebezpieczenwto: zbiezne wyspoy latwi kolonizuja p;ozostale, jesli sa lepsze
	Population population;
	Selection* selection;
	Crossover* crossover;
	Mutation* mutation;
	TimeCondition& endCondition;

	double current_best_fitness_;
	double current_mediana_fitness_;

	double eliteRatio = 0.001;

	int properSize;
	bool isIsolate = false;

	double startOfIsolate = 0.0;
	//uwaga mniejszy procent
	double maxRatioOfBest = 0.4;
	double ratioToCheckDiversity = 0.1;

	double lastShakeBestFitnes = 0;
	double lastShakeMedianaFitnes = 0;

	double lastWorseMigrationBestFitnes = 0;
	double lastWorseMigrationMedianaFitnes = 0;

	double lastResetBestFitnes = 0;

	//uwaga mienione!!!!!!!!!!!!!!!
	double ratioMigartionIfImprovment = 0.001;
	double lowerRatioMigartionIfStagnation = 0.01;
	double higherRatioMigartionIfStagnation = 0.05;

	double  ratioOfInydvidualToMigrationFromWorserIslandDown = 0.1;
	double  ratioOfInydvidualToMigrationFromWorserIslandUp = 0.4;

	double  ratioOfInydvidualToMigrationFromBetterIslandDown = 0.4;
	double  ratioOfInydvidualToMigrationFromBetterIslandUp = 0.6;

	double minMigartion = 2;

	double totalTime = 120;
	double shakeTermLength = 4.0 / totalTime * 100;
	double resetTermLengh = 10.0 / totalTime * 100;

	double nextShakeTime = shakeTermLength;
	double nextResetTime = resetTermLengh;

	double isolateTime = resetTermLengh;


};

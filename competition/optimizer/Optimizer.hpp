#pragma once

#include <vector>
#include <random>
#include "../templete/Evaluator.hpp"
#include "../population/population.hpp"
#include "../operations/selection.hpp"
#include "../operations/mutation.hpp"
#include "../operations/crossover.hpp"
#include <thread>
#include "../conditions/timeCondition.hpp"
#include "GA_Optimalizator.hpp"
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <atomic>
#include <limits>
#include <mutex>

using namespace std;

namespace LcVRPContest {
	class Optimizer {
	public:
		Optimizer(Evaluator& evaluator) : evaluator_(evaluator), numberIsland(6),
			fe(evaluator),
			current_best_fitness_(numeric_limits<double>::max()),
			current_mediana_fitness_((numeric_limits<double>::max())),
			endCondition(TimeCondition(2, 00, 0)) {
		}

		~Optimizer();

		
		void initForSmallProblems();
		void initForBigProblems();
		void initForMediumProblems();
		void initForMediumMatrixProblems();

		void Initialize();
		void RunIteration();

		vector<int>* GetCurrentBest() { return &current_best_; }
		double GetCurrentBestFitness() const { return current_best_fitness_; };

		void print();
		void printSummary();


	private:
		Evaluator& evaluator_;
		FitnessEvaluator fe;
		int numberIsland;

		vector<int> current_best_;
		double current_best_fitness_;

		double current_mediana_fitness_;
		TimeCondition endCondition;
		vector <GA_Optimalizator*> islands;
		vector<thread> threads;

		condition_variable cv1;
		mutex mut1;
		int endedLoops1=0;

		condition_variable cv3;
		mutex mut3;
		int endedLoops3=0;

		condition_variable cv2;
		mutex mut2;
		int endedLoops2=0;

		atomic<bool> isRun{ false };

		int worseTermLength = 50;
		int counter = 1;
		double momentToEnd = 0.96 * 100;
		double isRagnorok = false;
	};
}

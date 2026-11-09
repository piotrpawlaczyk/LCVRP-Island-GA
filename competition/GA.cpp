#include "runners.hpp"
#include "templete/Evaluator.hpp"
#include "optimizer/Optimizer.hpp"
#include "templete/ProblemLoader.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "conditions/timeCondition.hpp"

namespace GA {
	void StartOptimization(const string& folder_name, const string& instance_name, 
		bool use_random_permutation = true) {
		ProblemLoader problem_loader(folder_name, instance_name, use_random_permutation);
		ProblemData problem_data = problem_loader.LoadProblem();

		int num_groups = problem_data.GetNumGroups();
		Evaluator evaluator(problem_data, num_groups);
		Optimizer optimizer(evaluator);

		cout << "Start Initialize" << endl;
		optimizer.Initialize();
		cout << "End Initialize" << endl;

		EndCondition* endCondition = new TimeCondition(0, 0, 30);
		endCondition->start();
		for (int i = 0; !endCondition->chceckEndCondition(); i++) {
			optimizer.RunIteration();
			if (i % 10 == 0) {
				vector<int>* best_solution = optimizer.GetCurrentBest();
				cout << "loop: " << i;
				optimizer.print();
			}
		}

		optimizer.printSummary();

		vector<int>* best_solution = optimizer.GetCurrentBest();
		double best_fitness = evaluator.Evaluate(*best_solution);
		cout << "official best fitness: " << best_fitness << endl;
	}

	int creationMain() {
		bool use_random_permutation = true; // fixed seed for reproducibility, rand perm has more fair search space :)

		const string folder_name = "Vrp-Set-X";
		const string instance_name = "X-n209-k16";
		StartOptimization(folder_name, instance_name, use_random_permutation);

		return 0;
	}
}
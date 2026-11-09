#include <iostream>
#include <limits>
#include <iomanip>
#include "Optimizer.hpp"
#include "../population/routeFabric.hpp"
#include "../operations/tournamentSelection.hpp"
#include "../operations/routeCrossover.hpp"
#include "../operations/LNS.hpp"
#include "../operations/insertLocalSearch.hpp"
#include "../operations/insertMutation.hpp"
#include "../operations/LNS_Term.hpp"
#include "../operations/bigCrossover.hpp"
#include "../operations/routeImprovementCrossover.hpp"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <thread>

using namespace LcVRPContest;

Optimizer::~Optimizer() {
	isRun = false;

	{
		unique_lock<mutex> lk(mut3);
		endedLoops3++;
		if (endedLoops3 == numberIsland) {
			endedLoops3 = 0;
			cv3.notify_all();
		}
		else {
			cv3.wait(lk, [&] {return endedLoops3 == 0; });
		}
	}

	for (thread& t : threads)
		t.join();

	for (GA_Optimalizator* island : islands)
		delete island;
	islands.clear();
};

void Optimizer::Initialize() {
	endCondition.start();

	if (fe.getNumCustomers() < 110)
		initForSmallProblems();
	else if (fe.getNumCustomers()  < 1'510 && (fe.getEvaluator().GetProblemData().GetEdgeWeightType() == "EXPLICIT"))
		initForMediumMatrixProblems();
	else if (fe.getNumCustomers() < 1'410)
		initForMediumProblems();
	else 
		initForBigProblems();

	for (GA_Optimalizator* island : islands)
		island->Initialize();

	current_best_ = islands[0]->getBest()->toNumberedGroup();
	current_best_fitness_ = islands[0]->getBest()->getFitness();


	isRun = true;
	//first island is mine
	for (int i = 1; i < islands.size(); i++) {
		GA_Optimalizator* island = islands[i];
		threads.emplace_back([&, island] {
			{
				unique_lock<mutex> lk3(mut3);
				endedLoops3++;
				if (endedLoops3 == numberIsland) {
					endedLoops3 = 0;
					cv3.notify_all();
				}
				else {
					cv3.wait(lk3, [&] {return endedLoops3 == 0; });
				}
			}
			while (isRun) {
				{
					unique_lock<mutex> lk1(mut1);
					endedLoops1++;
					if (endedLoops1 == numberIsland) {
						endedLoops1 = 0;
						cv1.notify_all();
					}
					else {
						cv1.wait(lk1, [&] {return endedLoops1 == 0; });
					}
				}

				island->RunIteration();


				{
					unique_lock<mutex> lk2(mut2);
					endedLoops2++;
					if (endedLoops2 == numberIsland) {
						endedLoops2 = 0;
						cv2.notify_all();
					}
					else {
						cv2.wait(lk2, [&] {return endedLoops2 == 0; });
					}
				}

				{
					unique_lock<mutex> lk3(mut3);
					endedLoops3++;
					if (endedLoops3 == numberIsland) {
						endedLoops3 = 0;
						cv3.notify_all();
					}
					else {
						cv3.wait(lk3, [&] {return endedLoops3 == 0; });
					}
				}
			}
			});
	}
}

void Optimizer::RunIteration() {
	{
		unique_lock<mutex> lk3(mut3);
		endedLoops3++;
		if (endedLoops3 == numberIsland) {
			endedLoops3 = 0;
			cv3.notify_all();
		}
		else {
			cv3.wait(lk3, [&] {return endedLoops3 == 0; });
		}
	}
	{
		unique_lock<mutex> lk1(mut1);
		endedLoops1++;
		if (endedLoops1 == numberIsland) {
			endedLoops1 = 0;
			cv1.notify_all();
		}
		else {
			cv1.wait(lk1, [&] {return endedLoops1 == 0; });
		}
	}

	//runt iteration
	islands[0]->RunIteration();
	{
		unique_lock<mutex> lk2(mut2);
		endedLoops2++;
		if (endedLoops2 == numberIsland) {
			endedLoops2 = 0;
			cv2.notify_all();
		}
		else {
			cv2.wait(lk2, [&] {return endedLoops2 == 0; });
		}
	}

	//uwaga zmniana dla malych
	if ((++counter) % worseTermLength == 0 && counter > worseTermLength)
		for (GA_Optimalizator* island : islands)
			island->worseMigration(islands);

	/*
	if (counter % betterTermLength == 0)
		for (GA_Optimalizator* island : islands)
			island->betterMigration(islands);

	if (counter % shakeTermLengath == 0)
		for (GA_Optimalizator* island : islands)
			island->shakePopulation();
	*/
	if (!isRagnorok) {
		for (GA_Optimalizator* island : islands)
			island->reset(islands);
	}

	//if (counter % 10 == 0 && counter % worseTermLength != 0) {
	//	for (GA_Optimalizator* island : islands) {
	//		island->checkDiversity();
	//	}
	//}

	if (endCondition.percentComplet() > momentToEnd) {
		isRagnorok = true;
		for (GA_Optimalizator* island : islands)
			island->endMigration(islands);
	}
	if (endCondition.percentComplet() > 111 && isRagnorok) {
		isRagnorok = false;
		for (GA_Optimalizator* island : islands)
			island->fenixRises();
	}

	Indyvidual* bestIndyvidual = islands.front()->getBest();
	double bestFitness = bestIndyvidual->getFitness();
	double last_best_fitness = current_best_fitness_;
	for (GA_Optimalizator* island : islands) {
		if (bestFitness > island->getBest()->getFitness()) {
			bestIndyvidual = island->getBest();
			bestFitness = bestIndyvidual->getFitness();
		}
	}

	if (bestFitness + 0.01 < current_best_fitness_) {
		current_best_ = bestIndyvidual->toNumberedGroup();
		current_best_fitness_ = bestIndyvidual->getFitness();

	}
}


void Optimizer::initForSmallProblems() {
	int size = (3'000 + 1.0 / fe.getNumCustomers() * 100 * 2'000) / fe.getNumCustomers();

	RND* rnd0 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric0(fe, *rnd0);
	Population population(fabric0.initInidvidualsV2( 0.4 * size), *rnd0);
	GA_Optimalizator* opt0 = new GA_Optimalizator(fe, rnd0, population,
		new TournamentSelection(3, *rnd0),
		new RouteCrossover(fe, *rnd0, 0.6, 0.4),
		new LNS(0.2, true, false, false, false, *rnd0),
		endCondition
	);

	int s1 = size * 40;
	RND* rnd1 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric1(fe, *rnd1);
	Population population1(fabric1.creator(0, 0, s1, 0, false), *rnd1);
	GA_Optimalizator* opt1 = new GA_Optimalizator(fe, rnd1, population1,
		new TournamentSelection(3, *rnd1),
		new RouteImprovmentCrossover(fe, *rnd1, 0.7, 0.1),
		new Mutation(*rnd1),
		endCondition
	);

	//za malod zbiezny
	RND* rnd2 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric2(fe, *rnd2);
	Population population2(fabric2.initInidvidualsV2(size), *rnd2);
	GA_Optimalizator* opt2 = new GA_Optimalizator(fe, rnd2, population2,
		new TournamentSelection(3, *rnd2),
		new RouteCrossover(fe, *rnd2, 0.6, 0.1),
		new InsertMutation(0.01, 0.5, 0.01, *rnd2),
		endCondition
	);

	//MEGA podobny do najlepszego

	RND* rnd3 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric3(fe, *rnd3);
	Population population3(fabric3.initInidviduals(size*0.7), *rnd3);
	GA_Optimalizator* opt3 = new GA_Optimalizator(fe, rnd3, population3,
		new TournamentSelection(3, *rnd3),
		new RouteImprovmentCrossover(fe, *rnd3, 0.8, 0.2),
		new LNS(0.04, true, false, true, true, *rnd3),
		endCondition
	);

	//zdecydowamnie najlepszy, nie TYKAC!!!!!

	RND* rnd4 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric4(fe, *rnd4);
	Population population4(fabric4.initInidviduals( size), *rnd4);
	GA_Optimalizator* opt4 = new GA_Optimalizator(fe, rnd4, population4,
		new TournamentSelection(3, *rnd4),
		new RouteCrossover(fe, *rnd4, 0.6, 0.15),
		new LNS(0.01, true, false, true, true, *rnd4),
		endCondition
	);
	//MEGA podobny do najlepszego

	RND* rnd5 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric5(fe, *rnd5);
	Population population5(fabric5.initInidviduals(size*0.8), *rnd5);
	GA_Optimalizator* opt5 = new GA_Optimalizator(fe, rnd5, population5,
		new TournamentSelection(3, *rnd5),
		new RouteCrossover(fe, *rnd5, 0.6, 0.2),
		new LNS(0.05, true, false, true, true, *rnd5, 0.05),
		endCondition
	);
	worseTermLength = 500;

	islands = { opt0, opt1, opt2, opt3, opt4, opt5 };


	for (GA_Optimalizator* opt : islands)
		opt->setTerm(2.5, 1);
}

void Optimizer::initForBigProblems() {

	RND* rnd0 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric0(fe, *rnd0);
	// Leave room for two elite individuals and at least one pair of offspring.
	int size = max(4.0, 50.0 * pow(fe.getNumCustomers() / 4'000'000.0, 3));
	Population population0(fabric0.initGiantInidviduals(size), *rnd0);
	if(fe.getNumCustomers() < 10'000)
		for (int i = 0; i < size; i++)
			population0.get(i)->LNSlocalSearch(true, false, true, true);
	GA_Optimalizator* opt0 = new GA_Optimalizator(fe, rnd0, population0,
		new TournamentSelection(2, *rnd0),
		new RouteImprovmentCrossover(fe, *rnd0),
		new LNS(0.04, true, false, true, true, *rnd0),
		endCondition
	);

	//zdecydowamnie najlepszy, nie TYKAC!!!!!

	RND* rnd1 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric1(fe, *rnd1);
	Population population1(fabric1.initGiantInidviduals(size), *rnd1);
	if (fe.getNumCustomers() < 10'000)
		for (int i = 0; i < size; i++)
			population1.get(i)->LNSlocalSearch(true, false, true, true);
	GA_Optimalizator* opt1 = new GA_Optimalizator(fe, rnd1, population1,
		new TournamentSelection(2, *rnd1),
		new RouteImprovmentCrossover(fe, *rnd1),
		new LNS(0.04, true, false, true, false, *rnd1, 0.001),
		endCondition
	);

	RND* rnd2 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric2(fe, *rnd2);
	Population population2(fabric2.initGiantInidviduals(size), *rnd2);
	if (fe.getNumCustomers() < 10'000)
		for (int i = 0; i < size; i++)
			population2.get(i)->LNSlocalSearch(true, false, true, true, 0.001);
	GA_Optimalizator* opt2 = new GA_Optimalizator(fe, rnd2, population2,
		new TournamentSelection(2, *rnd2),
		new RouteImprovmentCrossover(fe, *rnd2, 0.6, 0.01, 0.7),
		new LNS(0.04, true, false, true, false, *rnd2, 0.0001),
		endCondition
	);

	RND* rnd3 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric3(fe, *rnd3);
	Population population3(fabric3.initGiantInidviduals(size), *rnd3);
	if (fe.getNumCustomers() < 10'000)
		for (int i = 0; i < size; i++)
			population3.get(i)->LNSlocalSearch(true, false, true, true, 0.001);
	GA_Optimalizator* opt3 = new GA_Optimalizator(fe, rnd3, population3,
		new TournamentSelection(2, *rnd3),
		new RouteImprovmentCrossover(fe, *rnd3, 0.6, 0.01, 0.7),
		new LNS(0.04, true, false, true, true, *rnd3),
		endCondition
	);

	//zdecydowamnie najlepszy, nie TYKAC!!!!!

	RND* rnd4 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric4(fe, *rnd4);
	Population population4(fabric4.initGiantInidviduals(size), *rnd4);
	if (fe.getNumCustomers() < 10'000)
		for (int i = 0; i < size; i++)
			population4.get(i)->LNSlocalSearch(true, false, true, true, 0.001);
	GA_Optimalizator* opt4 = new GA_Optimalizator(fe, rnd4, population4,
		new TournamentSelection(2, *rnd4),
		new RouteImprovmentCrossover(fe, *rnd4, 0.6, 0.01, 0.7),
		new LNS(0.04, true, false, true, true, *rnd4, 0.001),
		endCondition
	);

	RND* rnd5 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric5(fe, *rnd5);
	GA_Optimalizator* opt5;
	Population population5(fabric5.initGiantInidviduals(size), *rnd5);
	if (fe.getNumCustomers() < 10'000) {
		for (int i = 0; i < size; i++)
			population5.get(i)->LNSlocalSearch(true, false, true, true, 0.001);
		opt5 = new GA_Optimalizator(fe, rnd5, population5,
			new TournamentSelection(2, *rnd5),
			new RouteImprovmentCrossover(fe, *rnd5, 0.6, 0.01, 0.7),
			new LNS(0.04, true, false, true, true, *rnd5),
			endCondition
		);
	}
	else {
		population5.get(0)->LNSlocalSearch(true, false, false, false);
		opt5 = new GA_Optimalizator(fe, rnd5, population5,
			new TournamentSelection(2, *rnd5),
			new RouteImprovmentCrossover(fe, *rnd5, 0.6, 0.01, 0.7),
			new LNS(0.5, true, false, true, true, *rnd5),
			endCondition
		);
	}

	

	/*
	RND* rnd3 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric3(fe, *rnd3);
	Population population3(fabric3.initGiantInidviduals(40), *rnd3);
	GA_Optimalizator* opt3 = new GA_Optimalizator(fe, rnd3, population3,
		new TournamentSelection(2, *rnd3),
		new BigCrossover(fe, *rnd3, 12, counter),
		new LNS_Term(100, 0.2, 0.6, *rnd3, counter),
		endCondition, 30
	);


	RND* rnd4 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric4(fe, *rnd4);
	Population population4(fabric4.initGiantInidviduals(40), *rnd4);
	GA_Optimalizator* opt4 = new GA_Optimalizator(fe, rnd4, population4,
		new TournamentSelection(2, *rnd4),
		new BigCrossover(fe, *rnd4, 12, counter),
		new LNS_Term(10, 0.2, 0.6, *rnd4, counter),
		endCondition, 30
	);


	RND* rnd5 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric5(fe, *rnd5);
	Population population5(fabric5.initGiantInidviduals(40), *rnd5);
	GA_Optimalizator* opt5 = new GA_Optimalizator(fe, rnd5 , population5,
		new TournamentSelection(2, *rnd5),
		new BigCrossover(fe, *rnd5, 12, counter),
		new LNS_Term(10, 0.2, 0.6, *rnd5, counter),
		endCondition, 30
	);

	*/


	worseTermLength = 20;
	islands = { opt0, opt1, opt2, opt3, opt4, opt5 };
};

void Optimizer::initForMediumProblems() {
	double baseQsizePopulation = (50 + 110'000.0 / fe.getNumCustomers()) / fe.getNumCustomers();

	int sizeTournament;
	if (fe.getNumCustomers() > 800)
		sizeTournament = 2;
	else sizeTournament = 3;
	RND* rnd0 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric0(fe, *rnd0);
	Population population(fabric0.initInidvidualsV2(baseQsizePopulation), *rnd0);
	GA_Optimalizator* opt0 = new GA_Optimalizator(fe, rnd0, population,
		new TournamentSelection(sizeTournament, *rnd0),
		new RouteImprovmentCrossover(fe, *rnd0, 0.6, 0.001),
		new LNS(0.1, true, false, true, true, *rnd0),
		endCondition
	);

	RND* rnd1 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric1(fe, *rnd1);
	Population population1(fabric1.initInidviduals(baseQsizePopulation), *rnd1);
	GA_Optimalizator* opt1 = new GA_Optimalizator(fe, rnd1, population1,
		new TournamentSelection(sizeTournament, *rnd1),
		new RouteCrossover(fe, *rnd1, 0.6, 0.001, false),
		new LNS(0.1, true, false, true, true, *rnd1, 0.003),
		endCondition
	);

	RND* rnd2 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric2(fe, *rnd2);
	Population population2(fabric2.initInidvidualsV2(baseQsizePopulation*1.1), *rnd2);
	GA_Optimalizator* opt2 = new GA_Optimalizator(fe, rnd2, population2,
		new TournamentSelection(sizeTournament, *rnd2),
		new RouteCrossover(fe, *rnd2),
		new LNS(0.12, true, false, true, false, *rnd2),
		endCondition
	);

	RND* rnd3 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric3(fe, *rnd3);
	Population population3(fabric3.initInidviduals(baseQsizePopulation * 1.1), *rnd3);
	GA_Optimalizator* opt3 = new GA_Optimalizator(fe, rnd3, population3,
		new TournamentSelection(sizeTournament, *rnd3),
		new RouteCrossover(fe, *rnd3, 0.5, 0.1),
		new InsertLocalSearch(0.1, 1, *rnd3),
		endCondition
	);

	//zdecydowamnie najlepszy, nie TYKAC!!!!!
	RND* rnd4 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric4(fe, *rnd4);
	Population population4(fabric4.initInidviduals( baseQsizePopulation), *rnd4);
	GA_Optimalizator* opt4 = new GA_Optimalizator(fe, rnd4, population4,
		new TournamentSelection(sizeTournament, *rnd4),
		new RouteCrossover(fe, *rnd4, 0.6, 0.01,  false),
		new LNS(0.1, true, false, true, true, *rnd4),
		endCondition
	);
	
	RND* rnd5 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric5(fe, *rnd5);
	Population population5(fabric5.initInidviduals(baseQsizePopulation*1.7), *rnd5);
	GA_Optimalizator* opt5 = new GA_Optimalizator(fe, rnd5, population5,
		new TournamentSelection(sizeTournament+1, *rnd5),
		new RouteCrossover(fe, *rnd5, 0.6, 0.2),
		new LNS(0.05, true, false, true, false, *rnd5),
		endCondition
	);


	worseTermLength = 40;
	islands = { opt0, opt1, opt2, opt3, opt4, opt5 };

	//islands = { opt0 };
	//for (GA_Optimalizator* opt : islands)
	//	opt->setTerm(3, 10);
}

void Optimizer::initForMediumMatrixProblems() {
	double baseQsizePopulation = (20+200'000.0 / fe.getNumCustomers()) / fe.getNumCustomers();

	int sizeTournament;
	if (fe.getNumCustomers() > 800)
		sizeTournament = 2;
	else if (fe.getNumCustomers() > 600)
		sizeTournament = 3;
	else sizeTournament = 4;
	RND* rnd0 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric0(fe, *rnd0);
	Population population(fabric0.initInidvidualsV2(baseQsizePopulation), *rnd0);;
	GA_Optimalizator* opt0 = new GA_Optimalizator(fe, rnd0, population,
		new TournamentSelection(sizeTournament, *rnd0),
		new RouteImprovmentCrossover(fe, *rnd0, 0.7),
		new LNS(0.08, true, true, false, false, *rnd0),
		endCondition
	);

	RND* rnd1 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric1(fe, *rnd1);
	Population population1(fabric1.initInidvidualsV2(baseQsizePopulation*1.5), *rnd1);
	GA_Optimalizator* opt1 = new GA_Optimalizator(fe, rnd1, population1,
		new TournamentSelection(sizeTournament, *rnd1),
		new RouteCrossover(fe, *rnd1, 0.7, 0.1, false),
		new InsertMutation(0.1, 3.0 / fe.getNumGroups(), 0.1, *rnd1),
		endCondition
	);

	RND* rnd2 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric2(fe, *rnd2);
	Population population2(fabric2.initInidvidualsV2(baseQsizePopulation * 1.1), *rnd2);
	GA_Optimalizator* opt2 = new GA_Optimalizator(fe, rnd2, population2,
		new TournamentSelection(max(2, sizeTournament), *rnd2),
		new RouteImprovmentCrossover(fe, *rnd2, 0.5, 0.01),
		new LNS(0.3, true, false, false, false, *rnd2, 0.12),

		endCondition
	);

	RND* rnd3 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric3(fe, *rnd3);
	Population population3(fabric3.initInidviduals(baseQsizePopulation), *rnd3);
	GA_Optimalizator* opt3 = new GA_Optimalizator(fe, rnd3, population3,
		new TournamentSelection(sizeTournament, *rnd3),
		new RouteImprovmentCrossover(fe, *rnd3, 0.5, 0.1),
		new InsertLocalSearch(0.1, 1, *rnd3),
		endCondition
	);

	//zdecydowamnie najlepszy, nie TYKAC!!!!!
	RND* rnd4 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric4(fe, *rnd4);
	Population population4(fabric4.initInidviduals(baseQsizePopulation * 1.5), *rnd4);
	GA_Optimalizator* opt4 = new GA_Optimalizator(fe, rnd4, population4,
		new TournamentSelection(sizeTournament+1, *rnd4),
		new RouteImprovmentCrossover(fe, *rnd4, 0.7, 0.1),
		new LNS(0.01, true, false, true, true, *rnd4, 0.01),
		endCondition
	);

	RND* rnd5 = new RND(fe.getNumCustomers(), fe.getNumGroups());
	RouteFabric fabric5(fe, *rnd5);
	Population population5(fabric5.initInidviduals(baseQsizePopulation * 2 / 3), *rnd5);
	GA_Optimalizator* opt5 = new GA_Optimalizator(fe, rnd5, population5,
		new TournamentSelection(sizeTournament, *rnd5),
		new RouteImprovmentCrossover(fe, *rnd5),
		new LNS(0.08, true, false, true, true, *rnd5),
		endCondition
	);

	worseTermLength = 100;
	islands = { opt0, opt1, opt2, opt3, opt4, opt5 };
}

void Optimizer::print(){
	cout << "\n Current best fitness: " << current_best_fitness_ << '\n';

	for (int i=0;i < islands.size();i++) {
		cout << "Island " << i << " best fitness: " << islands[i]->getBest()->getFitness() << '\n';

	}
	cout << '\n';
}

void Optimizer::printSummary(){
	cout << "\n\n----------------------------\n";
	cout << "Summary of best solution:\n";

	cout << "Best fitness: " << current_best_fitness_ << '\n';

	GA_Optimalizator* bestIsland = islands.front();
	for (int i=0;i < islands.size();i++) {
		cout << "Island " << i << " best fitness: " << islands[i]->getBest()->getFitness() << '\n';
		if (islands[i]->getBest()->getFitness() < bestIsland->getBest()->getFitness())
			bestIsland = islands[i];
	}

	cout << "\nBest phenotype: ";
	for (int i = 0; i < current_best_.size(); i++)
		cout << current_best_[i] << ' ';
	cout << '\n';

	cout << "\nBest genotype: ";
	bestIsland->getBest()->print();
	cout << '\n';
}

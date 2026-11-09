#pragma once
#include "../templete/Evaluator.hpp"
#include <vector>
#include <random>
#include "../population/indyvidual.hpp"
#include <utility>
#include "../population/population.hpp"
#include "../utility/fitnessEvaluator.hpp"
#include <string>

using namespace std;
class Selection {
public:
	//Selection(FitnessEvaluator& fitnessEvaluator) : fitnessEvaluator(fitnessEvaluator) {};
	virtual ~Selection() = default;

	virtual void initializeSelection(Population* pop) = 0;

	//w przyszlosci mozna dac referencje i uniknac jednego z dwoch kopiowan
	virtual Indyvidual* getIndyvidual() = 0;
	virtual void bigSeletion() = 0;

	pair<Indyvidual*, Indyvidual*> getTwoIndyviduals(){	//zapytac sie o sndart na konkurs
		return pair<Indyvidual*, Indyvidual*> (getIndyvidual(), getIndyvidual() );
	}

	
	string& getName() { return name; }

protected:
	string name;
	//pyatnie czy to nie bedzie wolne
	Population* population;
	//FitnessEvaluator& fitnessEvaluator;
};
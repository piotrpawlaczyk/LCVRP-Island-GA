#include "LNS_Term.hpp"


LNS_Term::LNS_Term(int term, double  ratioPart, double ratioFull, RND& rnd, int& i): Mutation(rnd),
	term(term), ratioPart(ratioPart), ratioFull(ratioFull), i(i){
}

void LNS_Term::makeMuation(Indyvidual* ind) {
	if (i % term == 1) {
		if (rnd.randomBool(ratioFull))
			ind->LNSlocalSearch(true, false, true, true);
		else if (rnd.randomBool(ratioPart/(1.0 - ratioFull)) )
			ind->LNSlocalSearch(true, false, false, false);
	}
}
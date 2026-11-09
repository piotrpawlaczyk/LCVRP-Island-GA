#include "insertLocalSearch.hpp"


void InsertLocalSearch::makeMuation(Indyvidual* ind) {
	if (rnd.randomBool(probabilityMutationIndyvidual)) {
		if (rnd.randomBool(probabilityMutationIndyvidual))
			ind->LNSlocalSearch(true, false, false, false);
		else ind->LNSlocalSearch(true, false, true, true);

		ind->insertLocalSearch(probabilityMutationRoute);
	}
}
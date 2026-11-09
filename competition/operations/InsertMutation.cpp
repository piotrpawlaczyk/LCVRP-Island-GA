#include "insertMutation.hpp"



void InsertMutation::makeMuation(Indyvidual* ind) {
	if (rnd.randomBool(probabilityMutationIndyvidual))
		ind->insertMutation(probabilityMutationRoute, probabilityTOAcccpetWorse);
		
}

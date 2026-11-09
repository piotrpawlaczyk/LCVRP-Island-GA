#include "LNS.hpp"


LNS::LNS(double _baseRatioOfLS, bool _isOrderLS, bool _isReversLS,
	bool _isGreedyOrderLS, bool _isGreedyReversLS, RND& rnd, double probabilityWorser) :
	Mutation(rnd), probabilityWorser(probabilityWorser){
	baseRatioOfLS = _baseRatioOfLS;
	//preEvolutionRatioOfLS = _preEvolutionRatioOfLS;
	isOrderLS = _isOrderLS;
	isReversLS = _isReversLS;
	isGreedyOrderLS = _isGreedyOrderLS;
	isGreedyReversLS = _isGreedyReversLS;
}

/*
void LNS::makePreMuation(Population& pop) {
	for (Indyvidual* ind : pop.getAllIndyviduals())
		if (rnd::randomBool(preEvolutionRatioOfLS)) {
			ind->localSearch(isOrderLS, isReversLS, isGreedyOrderLS, isGreedyReversLS);
		}
	pop.finished();
}
*/
void LNS::makeMuation(Indyvidual* ind) {
	if (rnd.randomBool(baseRatioOfLS)) 
		ind->LNSlocalSearch(isOrderLS, isReversLS, isGreedyOrderLS, isGreedyReversLS, 
			probabilityWorser);
	
}

#pragma once

#include "mutation.hpp"


class InsertLocalSearch : public Mutation {
public:
	InsertLocalSearch(double probabilityMutationIndyvidual, double probabilityMutationRoute
		, RND& rnd) : Mutation(rnd),
		probabilityMutationIndyvidual(probabilityMutationIndyvidual),
		probabilityMutationRoute(probabilityMutationRoute)
	{}
	~InsertLocalSearch() override = default;
	//void makePreMuation(Population& pop) override;
	void makeMuation(Indyvidual* ind) override;

private:
	double probabilityMutationIndyvidual;
	double probabilityMutationRoute;
};
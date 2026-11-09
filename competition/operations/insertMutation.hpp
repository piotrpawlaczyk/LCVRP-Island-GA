#pragma once

#include "mutation.hpp"


class InsertMutation : public Mutation{
public:
	InsertMutation(double probabilityMutationIndyvidual, double probabilityMutationRoute, 
		double probabilityTOAcccpetWorse, RND& rnd): Mutation(rnd),
		probabilityMutationIndyvidual(probabilityMutationIndyvidual),
		probabilityMutationRoute(probabilityMutationRoute), probabilityTOAcccpetWorse(probabilityTOAcccpetWorse)
	{}
	~InsertMutation() override = default;
	//void makePreMuation(Population& pop) override;
	virtual void makeMuation(Indyvidual* ind) override;

private:
	double probabilityMutationIndyvidual;
	double probabilityMutationRoute;
	double probabilityTOAcccpetWorse;

};
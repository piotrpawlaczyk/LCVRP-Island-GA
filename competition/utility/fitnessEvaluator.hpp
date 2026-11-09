#pragma once

#include <vector>
#include "rnd.hpp"
#include "../templete/Evaluator.hpp"
#include <cassert>
#include <cstddef>
using namespace LcVRPContest;

class FitnessEvaluator {
public:
	FitnessEvaluator(Evaluator& e) :
		evaluator(e), indexToCustomer(data.GetPermutation()), data(evaluator.GetProblemData()),
		demandsCustomers(evaluator.GetProblemData().GetDemands()){
		distanceMatrix = data.GetEdgeWeights();

		for (int i = 0; i < indexToCustomer.size(); i++) {
			indexToCustomer[i] -= 1;
		}

		for (int i = 0; i < indexToCustomer.size(); i++) {
			startsDistances.push_back(distanceMatrix[0][indexToCustomer[i]]);
			endDistances.push_back(distanceMatrix[0][indexToCustomer[i]]);

		}

		for (int i = 0; i < indexToCustomer.size(); i++) {
			distanceMatrixIndexes.push_back({});
			for (int j = 0; j < indexToCustomer.size(); j++) {
				distanceMatrixIndexes[i].push_back(distanceMatrix[indexToCustomer[i]][indexToCustomer[j]]);
			}
		}
		for (int i = 0; i < indexToCustomer.size(); i++) {
			demandsIndexes.push_back(demandsCustomers[indexToCustomer[i]]);
		}

		for (int i = 0; i < getNumGroups(); i++) {
			groupIndexesVector.push_back(i);
		}
		for (int i = 0; i < getNumCustomers(); i++) {
			customerIndexesVector.push_back(i);
		}

	}


	double getIndexDistance(int index1, int index2) const {
		return distanceMatrixIndexes[index1][index2];
	}
	double getIndexDistanceWithDepot(int index1, int index2) const {
		if (index1 < 0)
			return startsDistances[index2];
		else if (index2 < 0 || index2 >= getNumCustomers())
			return endDistances[index1];

		return distanceMatrixIndexes[index1][index2];
	}

	double getStart(int index)const {
		return startsDistances[index];
	}
	double getEnd(size_t index) const {
		return endDistances[index];

	}

	double getDepotX() {
		return data.GetCoordinates()[0].x;
	}

	double getDepotY() {
		return data.GetCoordinates()[0].y;
	}

	Coordinate getCoordinate(int indexCustomers) {
		return data.GetCoordinates()[indexToCustomer[indexCustomers]];
	}

	bool validConstraints(double distance, double capacity, bool isFirst,
		int lastIndex, int newIndex) {
		return capacity + getDemandsIndex(newIndex) <= getMaxCapacity() &&
			(isFirst || !data.HasDistanceConstraint() ||
				distance + getIndexDistance(lastIndex, newIndex) +
				getEnd(newIndex) < data.GetDistance());
	}


	const std::vector<int>& getIndexToCustomer() const {
		return indexToCustomer;
	}

	int getDemandsIndex(int index) const {
		return demandsIndexes[index];
	}

	int getNumCustomers()const {
		return data.GetNumCustomers();
	}
	int getNumGroups() const {
		return evaluator.GetNumGroups();
	}
	double getMaxCapacity() const {
		return data.GetCapacity();
	}

	double getMaxDistance() const {
		return data.GetDistance();
	}


	bool hasDistanceConstraint() {
		return data.HasDistanceConstraint();
	}
	Evaluator& getEvaluator() const { return evaluator; }

	vector<int> getGroupIndexesVector() {
		return groupIndexesVector;
	}
	vector<int> getCustomerIndexesVector() {
		return customerIndexesVector;
	}


private:

	/*double getCustomersDistance(size_t customer1, size_t customer2)const {
		return distanceMatrix[customer1][customer2];
	}*/
	Evaluator& evaluator;
	const ProblemData& data;
	std::vector<std::vector<double>> distanceMatrix;
	std::vector<std::vector<double>> distanceMatrixIndexes;
	vector<double> startsDistances;
	vector<double> endDistances;

	std::vector<int> indexToCustomer;

	const vector<int>& demandsCustomers;
	vector<int> demandsIndexes;


	std::vector<int> groupIndexesVector;
	std::vector<int> customerIndexesVector;
};
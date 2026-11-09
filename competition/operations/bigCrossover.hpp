#pragma once
#include "crossover.hpp"
#include "../population/routeFabric.hpp"
#include <algorithm>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

using namespace std;
class BigCrossover : public Crossover {
public:
    //double mutationProb = 0.001,
    //usunac klonowanie !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    virtual pair<Indyvidual*, Indyvidual*> croosover(Indyvidual* ind1, Indyvidual* ind2) {

        if (i % crossoverTerm == 0)
            return { createIndyvidual(ind1, ind2), createIndyvidual(ind2, ind1) };
        else
            return{ createIndyvidual(ind1->clone()), createIndyvidual(ind2->clone()) };
    }

    BigCrossover(FitnessEvaluator& fe, RND& rnd, int crossoverTerm , int &i) : Crossover(fe, rnd, 0),
    i(i), crossoverTerm(crossoverTerm){ }
    
    virtual Indyvidual* createIndyvidual(Indyvidual* i1) {
        if (fe.getNumGroups() <  4)
            return i1;
        int numberGroupFrom1 = rnd.getRandomGroup();
        int numberGroupFrom2;
        int numberGroupTo1;
        int numberGroupTo2;
        do {
            numberGroupFrom2 = rnd.getRandomGroup();
        } while (numberGroupFrom2 == numberGroupFrom1);

        do {
            numberGroupTo1 = rnd.getRandomGroup();
        } while (numberGroupTo1 == numberGroupFrom1 || numberGroupTo1 == numberGroupFrom2 );

        do {
            numberGroupTo2 = rnd.getRandomGroup();
        } while (
            numberGroupTo2 == numberGroupFrom1 ||
            numberGroupTo2 == numberGroupFrom2 ||
            numberGroupTo2 == numberGroupTo1
            );

        i1->escapeMuation(numberGroupFrom1, numberGroupFrom2, numberGroupTo1, numberGroupTo2);
        //i1->escapeMuation(numberGroupFrom1, numberGroupFrom2);
        return i1;
    }
    virtual Indyvidual* createIndyvidual(Indyvidual* i1, Indyvidual* i2) override {
        //Indyvidual baseInd1 = *i1;
        RoutesIndyvidual* baseInd = dynamic_cast<RoutesIndyvidual*>(i1);
        RoutesIndyvidual* addInd = dynamic_cast<RoutesIndyvidual*>(i2);

        //uwana na przeciazenie
        vector<Routes> baseGroups = baseInd->getGroup();
        vector<Routes> addGroups = addInd->getGroup();
        vector<Routes> newGroups;

        double numberGroup = fe.getNumGroups();
        double numberCustomers = fe.getNumCustomers();

        vector<bool>isVisit;
        isVisit.resize(numberCustomers, false);


        mt19937& rand = rnd.rng();
        std::bernoulli_distribution move{ 0.4 };

        //zachowanie claych grup
        // Czy na pewno dobrze routes jest kopjowane?
        for (Routes& group : baseGroups) {
            if (move(rand)) {
                for (int customer : group.toVector())
                    isVisit[customer] = true;
                group.activeRoute();
                newGroups.push_back(group);

            }
        }

        if (newGroups.size() + 1 >= baseGroups.size())
            return new RoutesIndyvidual(*baseInd);

        int firstEmptyGroupIndex = newGroups.size();
        int lastEmptyGroupIndex = numberGroup - 1;
        std::uniform_int_distribution<int> indexGroupGen{ firstEmptyGroupIndex, lastEmptyGroupIndex };

        for (int i = firstEmptyGroupIndex; i < fe.getNumGroups(); i++) {
            newGroups.push_back(Routes(fe, rnd));
        }

        int nullVal = -1;

        //vector of cities whuich have edge with depot
        deque<int> cityFromDepot;
        for (Routes& group : addGroups) {
            vector<int> cityFromDepotOfGroup = group.getFirstCities();
            cityFromDepot.insert(cityFromDepot.end(), cityFromDepotOfGroup.begin(), cityFromDepotOfGroup.end());
        }
        sort(cityFromDepot.begin(), cityFromDepot.end());

        //nastpmny oczekiwany klient na ktory czeka grupa
        vector<int> nextCustomer;
        nextCustomer.resize(fe.getNumGroups(), nullVal);
        //mozna te pozostale takze zainicjowac
        for (int i = firstEmptyGroupIndex; i < fe.getNumGroups() && !cityFromDepot.empty(); i++) {
            nextCustomer[i] = cityFromDepot.front();
            cityFromDepot.pop_front();
        }

        //edgeFromParent[i] = j, krawedz i do j z rodzica added
        vector<int> edgeFromParent;
        edgeFromParent.resize(fe.getNumCustomers(), nullVal);
        for (Routes& group : addGroups) {
            vector <int> customersIndexes = group.toVector();
            for (int i = 1; i < customersIndexes.size(); i++) {
                if (!isVisit[customersIndexes[i - 1]] && !isVisit[customersIndexes[i]]) {
                    edgeFromParent[customersIndexes[i - 1]] = customersIndexes[i];
                }
            }
        }

        bool isSame = (baseInd->getFitness() == addInd->getFitness());
        int countEdge = 0;
        int countRandom = 0;
        RouteFabric fabric{ fe, rnd };
        //waskie gardlo algorytmu!!!! (maxymalna optymalziacja w 
        //przyszlosci,aktual nie waznw by dzialalo
        for (int customerIndex = 0; customerIndex < fe.getNumCustomers(); customerIndex++) {
            if (!isVisit[customerIndex]) {
                vector<int>::iterator it = find(nextCustomer.begin(), nextCustomer.end(), customerIndex);
                int groupIndex;
                //mutacja
                if (it != nextCustomer.end() && (rnd.randomBool(probOfMaintainPath) /*
                    || (isSame || rnd::randomBool(1. / fe.getNumCustomers()))*/)) {
                    groupIndex = distance(nextCustomer.begin(), it);
                    newGroups[groupIndex].addIndex(customerIndex);
                    countEdge++;
                }
                else {
                    //jkis mehcanizm ze dopeiro pod koniec
                    groupIndex = fabric.addToSelectedGroup(newGroups, customerIndex,
                        0, true);
                    countRandom++;
                }
                if (edgeFromParent[customerIndex] != nullVal) {
                    nextCustomer[groupIndex] = edgeFromParent[customerIndex];
                }
                //poki co tak, zponiej pomysle nad czyms innym
                else {
                    nextCustomer[groupIndex] = nullVal;
                }
            }
        }

        //uwaga na ponowne alczenie ciezarowki
        for (int i = 0; i < fe.getNumGroups(); i++)
            newGroups[i].closeRoute();


        Indyvidual* ind = new RoutesIndyvidual(fe, rnd, newGroups);

        return ind;

    }

private:
    int crossoverTerm;
    int& i;
    double probOfMaintainPath = 0.999;
};



#pragma once
#include "crossover.hpp"
#include "../population/routeFabric.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

using namespace std;
class RouteCrossover : public Crossover {
public:
    //double mutationProb = 0.001,

    virtual pair<Indyvidual*, Indyvidual*> croosover(Indyvidual* ind1, Indyvidual* ind2) {
        //check

        if (isRuinCrossover(rnd.rng()))
            return { createIndyvidual(ind1, ind2), createIndyvidual(ind2, ind1) };
        else if (isTryEscape && ind1->getFitness() == ind2->getFitness()){
            Indyvidual* newInd1 = ind1->clone();
            Indyvidual* newInd2 = ind2->clone();
            //uwaga ta wersja byla wczesniej w malej
            // 
            newInd1->swapMutation( 1.0/fe.getNumGroups() * 2 , ratioToSwap2, 0.5);
            //int rndGroup1 = rnd.getRandomGroup(); ;
            //int rndGroup2 = rnd.randomGroupIndexOtherThan(rndGroup1);
            //newInd1->escapeMuation(rndGroup1, rndGroup2);
            newInd2->randomRoutes(0.6);
            return { newInd1, newInd2 };
        };
        return { ind1->clone(), ind2->clone() };
    }

    
    virtual void randomize() override {
        //probOfMaintainPath = max(0.4, 
        //    (probOfMaintainPath * probOfMaintainPath + probOfMaintainPath * 0.9) / 2 );
        porbabilityOfMainatainRoutes = 0.6;
        ratioToSwap1 = 1.0 / sqrt(fe.getNumGroups());
        ratioToSwap2 = (1.0 / fe.getNumCustomers() * fe.getNumGroups()) + 0.5;
        //increaseCrossoverRate();
    }
    
    virtual void increaseDiversity() override {
        //probOfMaintainPath = max(0.4, 
        //    (probOfMaintainPath * probOfMaintainPath + probOfMaintainPath * 0.9) / 2 );
        //porbabilityOfMainatainRoutes = max(0.2, porbabilityOfMainatainRoutes * 0.9);
        //ratioToSwap1  = min(ratioToSwap1 * 1.1 , 1.0 / sqrt( fe.getNumCustomers() ));
        //ratioToSwap2 = min(0.8, ratioToSwap2 * 1.1);
        //increaseCrossoverRate();
    }

    RouteCrossover(FitnessEvaluator& fe, RND& rnd, double isCroosover = 0.6, double mutationProb = 0.01,
        bool isTryEscape = true) : probOfMaintainPath(1- mutationProb),
        Crossover(fe, rnd, isCroosover), isTryEscape(isTryEscape) {
        ratioToSwap1 = 1.0 / fe.getNumGroups() * 2;
        ratioToSwap2 = (1.0 / fe.getNumCustomers() * fe.getNumGroups()) + 0.5;
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
        std::bernoulli_distribution move{ porbabilityOfMainatainRoutes };

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

        //prevosu this was not emplace
        for (int i = firstEmptyGroupIndex; i < fe.getNumGroups(); i++) {
            newGroups.emplace_back(fe, rnd);
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
        RouteFabric fabric{ fe, rnd};
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
                       0, true, true);
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
        

        Indyvidual* ind =  new RoutesIndyvidual(fe, rnd, newGroups);

        return ind;

    }

private:
    double probOfMaintainPath;
    double porbabilityOfMainatainRoutes = 0.6;
    double ratioToSwap1;
    double ratioToSwap2;
    bool isTryEscape;
};



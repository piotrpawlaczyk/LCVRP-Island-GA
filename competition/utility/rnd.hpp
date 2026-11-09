#pragma once
#include <random>


class RND {
public:
    RND(int numCustomers, int numGroup) : seed(++id), rng_{ seed },
        rndGroup(0, numGroup - 1), rndCustomers(0, numCustomers - 1) { }

    int getRandomGroup() { return rndGroup(rng_); }
    int getRandomCustomers() { return rndCustomers(rng_); }

    bool randomBool(double probabilityTrue);
    int rndTo(int limit);
    int randomGroupIndexOtherThan(int otherIndex);
    std::mt19937& rng() { return rng_; }

private:
    static unsigned int id;
    unsigned int seed;
    std::mt19937 rng_{ seed };
    std::uniform_int_distribution<int> rndGroup;
    std::uniform_int_distribution<int> rndCustomers;
};
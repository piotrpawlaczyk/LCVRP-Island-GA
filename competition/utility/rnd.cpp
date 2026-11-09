#include "rnd.hpp"
#include <random>

bool RND::randomBool(double probabilityTrue) {
    if (probabilityTrue > 1)
        return true;
    if (probabilityTrue < 0)
        return false;
    std::bernoulli_distribution dist(probabilityTrue);
    return  dist(rng_);;
}

int RND::rndTo(int limit) {
    std::uniform_int_distribution<int> dist{ 0, limit };
    return dist(rng_);
}

int RND::randomGroupIndexOtherThan(int otherIndex) {
    int index = rndGroup(rng_);
    while(index == otherIndex)
        index = rndGroup(rng_);
    return index;
}

unsigned int RND::id = 0;
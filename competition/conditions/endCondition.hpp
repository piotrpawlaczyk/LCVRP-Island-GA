#pragma once
#include <cmath>

class EndCondition {
public:
    virtual ~EndCondition() = default;
                                        
    virtual void start() = 0;
    virtual bool isEnd(double diffrents) { return abs(diffrents) < 1 || chceckEndCondition(); }
    virtual double percentComplet() = 0;
    virtual bool chceckEndCondition() = 0;

};

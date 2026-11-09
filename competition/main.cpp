#include "runners.hpp"

constexpr bool isCompetitionVersion = true;
int main(){
    if(isCompetitionVersion)
        return competitionMain();
    else
        return GA::creationMain();
}

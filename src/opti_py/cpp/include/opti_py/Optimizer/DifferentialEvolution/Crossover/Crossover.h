#ifndef CROSSOVER_H
#define CROSSOVER_H

#include <vector>

#include "External/mt.h"

class Crossover {
public:
    virtual ~Crossover() = default;

    virtual void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) = 0;
};

#endif

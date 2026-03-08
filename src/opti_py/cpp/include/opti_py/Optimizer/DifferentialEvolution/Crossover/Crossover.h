#ifndef CROSSOVER_H
#define CROSSOVER_H

#include <vector>

#include "SolutionBuilder.h"

class Crossover {
public:
    virtual ~Crossover() = default;

    virtual void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double CR,
        SolutionBuilder& builder
    ) = 0;
};

#endif

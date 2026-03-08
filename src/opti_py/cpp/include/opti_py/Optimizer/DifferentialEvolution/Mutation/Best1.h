#ifndef BEST1_H
#define BEST1_H


#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"
#include <algorithm>


class Best1 : public Mutation {
public:
    std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        size_t targetIndex,
        double F,
        const std::vector<double>& bestVector,
        MersenneTwister& mt,
        double lowerBound,
        double upperBound
    ) override {
        std::vector<size_t> subset = getSubset(population.size(), 2, targetIndex, mt);

        const std::vector<double>& xr1 = population[subset[0]];
        const std::vector<double>& xr2 = population[subset[1]];

        std::vector<double> mutated(bestVector.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = bestVector[j] + F * (xr1[j] - xr2[j]);
            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }
        
        return mutated;
    }
};



#endif

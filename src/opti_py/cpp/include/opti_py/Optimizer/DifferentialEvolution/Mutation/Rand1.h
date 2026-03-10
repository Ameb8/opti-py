#ifndef RAND1_H
#define RAND1_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


class Rand1 : public Mutation {
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
        std::vector<size_t> subset = getSubset(population.size(), 3, targetIndex, mt);

        // Get population subset        
        const std::vector<double>& r1 = population[subset[0]];
        const std::vector<double>& r2 = population[subset[1]];
        const std::vector<double>& r3 = population[subset[2]];

        std::vector<double> mutated(bestVector.size());

        // Create mutated vector
        for(int i = 0; i < mutated.size(); i++) {
            mutated[i] = population[subset[1]][i] - population[subset[2]][i];
            mutated[i] *= F;
            mutated[i] += population[subset[0]][i];
            mutated[i] = std::clamp(mutated[i], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif

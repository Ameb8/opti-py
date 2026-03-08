#ifndef BEST2_H
#define BEST2_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


class Best2 : public Mutation {
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
        
        std::vector<size_t> subset = getSubset(population.size(), 4, targetIndex, mt);

        // Get population subset        
        const std::vector<double>& r1 = population[subset[0]];
        const std::vector<double>& r2 = population[subset[1]];
        const std::vector<double>& r3 = population[subset[2]];
        const std::vector<double>& r4 = population[subset[3]];


        std::vector<double> mutated(bestVector.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = bestVector[j]
                        + F * (r1[j] + r2[j] - r3[j] - r4[j]);

            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }

        return mutated;
    }
};


#endif

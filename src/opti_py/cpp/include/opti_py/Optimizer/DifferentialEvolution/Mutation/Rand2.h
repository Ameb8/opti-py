#ifndef RAND2_H
#define RAND2_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


class Rand2 : public Mutation {
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
        std::vector<size_t> subset = getSubset(population.size(), 5, targetIndex, mt);

        // Get population subset        
        const std::vector<double>& r1 = population[subset[0]];
        const std::vector<double>& r2 = population[subset[1]];
        const std::vector<double>& r3 = population[subset[2]];
        const std::vector<double>& r4 = population[subset[3]];
        const std::vector<double>& r5 = population[subset[4]];

        std::vector<double> mutated(r5.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = r5[j] 
                + F * (r1[j] - r2[j]) 
                + F * (r3[j] - r4[j]);

            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif

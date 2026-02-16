#ifndef BEST1_H
#define BEST1_H


#include "Optimizer/Mutation/Mutation.h"


class Best1 : public Mutation {
public:
    std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        int targetIndex,
        double F,
        const std::vector<double>& bestVector,
        SolutionBuilder& builder
    ) override {
        std::vector<int> subset = builder.getSubset(population.size(), 2, targetIndex);

        const std::vector<double>& xr1 = population[subset[0]];
        const std::vector<double>& xr2 = population[subset[1]];

        std::vector<double> mutated(bestVector.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = bestVector[j] + F * (xr1[j] - xr2[j]);
            mutated[j] = builder.checkBounds(mutated[j]);
        }
        
        return mutated;
    }
};


#endif

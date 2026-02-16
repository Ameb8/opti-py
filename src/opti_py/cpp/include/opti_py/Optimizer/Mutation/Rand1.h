#ifndef RAND1_H
#define RAND1_H

#include "Optimizer/Mutation/Mutation.h"

class Rand1 : public Mutation {
public:
    std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        int targetIndex,
        double F,
        const std::vector<double>& bestVector,
        SolutionBuilder& builder
    ) override {
        std::vector<int> subset = builder.getSubset(population.size(), 3, targetIndex);
    
        std::vector<double> mutated(population[targetIndex].size());

        // Create mutated vector
        for(int i = 0; i < mutated.size(); i++) {
            mutated[i] = population[subset[1]][i] - population[subset[2]][i];
            mutated[i] *= F;
            mutated[i] += population[subset[0]][i];
            mutated[i] = builder.checkBounds(mutated[i]);
        }

        return mutated;
    }
};

#endif

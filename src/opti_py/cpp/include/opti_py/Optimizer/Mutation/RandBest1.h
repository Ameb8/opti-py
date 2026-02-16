#ifndef RANDBEST1_H
#define RANDBEST1_H

#include "Optimizer/Mutation/Mutation.h"

class RandBest1 : public Mutation {
private:
    double lambda_;
public:
    explicit RandBest1(double lambda)
            : lambda_(lambda) {}

    std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        int targetIndex,
        double F,
        const std::vector<double>& bestVector,
        SolutionBuilder& builder
    ) override {
        std::vector<int> subset =
            builder.getSubset(population.size(), 2, targetIndex);

        const auto& xi  = population[targetIndex];
        const auto& r1  = population[subset[0]];
        const auto& r2  = population[subset[1]];

        std::vector<double> mutated(xi.size());

        for (int j = 0; j < mutated.size(); j++) {
            mutated[j] =
                xi[j]
                + lambda_ * (bestVector[j] - xi[j])
                + F * (r1[j] - r2[j]);

            mutated[j] = builder.checkBounds(mutated[j]);
        }

        return mutated;
    }
};

#endif 

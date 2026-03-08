#ifndef RANDBEST1_H
#define RANDBEST1_H


#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


class RandBest1 : public Mutation {
private:
    double lambda_;
public:
    explicit RandBest1(double lambda)
            : lambda_(lambda) {}

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

        // Get population subset        
        const auto& xi  = population[targetIndex];
        const auto& r1  = population[subset[0]];
        const auto& r2  = population[subset[1]];

        std::vector<double> mutated(xi.size());

        for (int i = 0; i < mutated.size(); i++) {
            mutated[i] =
                xi[i]
                + lambda_ * (bestVector[i] - xi[i])
                + F * (r1[i] - r2[i]);

            mutated[i] = std::clamp(mutated[i], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif 

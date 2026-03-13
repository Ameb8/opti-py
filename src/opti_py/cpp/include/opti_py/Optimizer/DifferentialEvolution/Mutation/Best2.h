/**
 * @file Best2.h
 * @author Alex Buckley
 * @brief DE/best/2 mutation strategy implementation.
 * @ingroup Mutation
 *
 * Implements the DE/best/2 mutation operator, which applies two weighted
 * difference vectors to the best (lowest fitness) individual found so far.
 */


#ifndef BEST2_H
#define BEST2_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


class Best2 : public Mutation {
public:


    /**
     * @brief Generates a mutant vector using DE/best/2 strategy.
     *
     * Applies the formula: mutant = best + F * (r1 - r2) + F * (r3 - r4)
     * where r1, r2, r3, r4 are four randomly selected distinct individuals.
     *
     * @param[in] population Current population of solutions
     * @param[in] targetIndex Index of the target individual (excluded from selection)
     * @param[in] F Differential weight; typical range 0.5-1.0
     * @param[in] bestVector The best solution found so far (used as base)
     * @param[in,out] mt Random number generator for selection
     * @param[in] lowerBound Lower domain bound
     * @param[in] upperBound Upper domain bound
     *
     * @return Mutant vector with all values clamped to [lowerBound, upperBound]
     */
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
                + F * (r1[j] - r2[j]) 
                + F * (r3[j] - r4[j]);

            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }

        return mutated;
    }
};


#endif

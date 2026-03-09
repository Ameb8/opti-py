#ifndef MUTATION_H
#define MUTATION_H

#include <vector>
#include <cstddef>

#include "External/mt.h"


class Mutation {
public:
    virtual ~Mutation() = default;

    virtual std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        size_t targetIndex,
        double F,
        const std::vector<double>& bestVector,
        MersenneTwister& mt,
        double lowerBound,
        double upperBound
    ) = 0;

protected:
    /**
     * @brief Select a random subset of indices from a population excluding the source index
     * 
     * @param populationSize Total number of individuals in the population
     * @param subsetSize Number of indices to select
     * @param source Index to exclude from selection
     * @param mt Random number generator
     * @return std::vector<size_t> Random subset of indices
     */
    std::vector<size_t> getSubset(
        size_t populationSize,
        size_t subsetSize,
        size_t source,
        MersenneTwister& mt
    ) {
        std::vector<size_t> indices(populationSize - 1);
        size_t idx = 0;

        // Prepare vector of all valid indices except source
        for(size_t i = 0; i < populationSize; i++) {
            if(i != source) {
                indices[idx] = i;
                ++idx;
            }
        }

        // Partial Fisher-Yates shuffle to select subsetSize random indices
        for(size_t i = 0; i < subsetSize; i++) {
            int j = i + (mt.genrand_int32() % (indices.size() - i));
            std::swap(indices[i], indices[j]);
        }

        // Return only the first 'subsetSize' indices
        indices.resize(subsetSize);
        return indices;
    }
};

#endif
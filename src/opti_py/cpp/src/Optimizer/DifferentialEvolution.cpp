#include "Optimizer/DifferentialEvolution/DifferentialEvolution.h"

#include <limits>
#include <stdexcept>


std::vector<size_t> DifferentialEvolution::getSubset(
    size_t populationSize,
    size_t subsetSize,
    size_t source,
    MersenneTwister& mt
) {
    std::vector<size_t> indices(populationSize - 1);
    size_t idx = 0;

    // Prepare vector of all valid indices except source
    for(size_t i = 0; i < populationSize; i++) {
        if (i != source) {
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

void DifferentialEvolution::crossover(
    std::vector<double>& target,
    const std::vector<double>& mutant,
    double cr,
    MersenneTwister& mt
) {
    int jrand = mt.genrand_int32() % target.size();

    for(size_t i = 0; i < target.size(); i++) {
        if (i == jrand || mt.genrand_real() < cr)
            target[i] = mutant[i];
    }
}


void DifferentialEvolution::clampValue(
    double& value, 
    double lowerBound, 
    double upperBound
) {
    if(value < lowerBound)
        value = lowerBound;
    else if(value > upperBound)
        value = upperBound;
}


std::vector<double> DifferentialEvolution::mutate(
    const std::vector<std::vector<double>>& population,
    const std::vector<size_t>& subset,
    double f,
    double lowerBound,
    double upperBound
) {
    std::vector<double> mutant(population[0].size());

    // Create mutated vector
    for(size_t i = 0; i < mutant.size(); i++) {
        mutant[i] = population[subset[1]][i] - population[subset[2]][i];
        mutant[i] *= f;
        mutant[i] += population[subset[0]][i];
        clampValue(mutant[i], lowerBound, upperBound);
    }

    return mutant;
}


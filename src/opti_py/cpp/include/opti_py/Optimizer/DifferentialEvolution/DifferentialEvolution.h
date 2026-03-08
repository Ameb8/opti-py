#ifndef DIFFERENTIAL_EVOLUTION_H
#define DIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <cstddef>
#include "External/mt.h"
#include "Evaluable.h"

class DifferentialEvolution {
public:
    DifferentialEvolution() = default;
    ~DifferentialEvolution() = default;

    /**
     * @brief Select a random subset of indices from a population excluding the source index
     * 
     * @param populationSize Total number of individuals in the population
     * @param subsetSize Number of indices to select
     * @param source Index to exclude from selection
     * @param mt Random number generator
     * @return std::vector<size_t> Random subset of indices
     */
    static std::vector<size_t> getSubset(
        size_t populationSize,
        size_t subsetSize,
        size_t source,
        MersenneTwister& mt
    );

    /**
     * @brief Perform DE crossover between target and mutant vector
     * 
     * @param target Vector to be modified
     * @param mutant Mutant vector used for crossover
     * @param cr Crossover rate
     * @param mt Random number generator
     */
    static void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    );

    /**
     * @brief Clamp a value within given bounds
     * 
     * @param value Reference to the value to clamp
     * @param lowerBound Minimum allowed value
     * @param upperBound Maximum allowed value
     */
    static void clampValue(
        double& value, 
        double lowerBound, 
        double upperBound
    );

    /**
     * @brief Generate a mutant vector for DE
     * 
     * @param population Current population of solutions
     * @param subset Indices of individuals used to generate mutant
     * @param f Differential weight
     * @param lowerBound Minimum allowed value
     * @param upperBound Maximum allowed value
     * @return std::vector<double> Mutant vector
     */
    static std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        const std::vector<size_t>& subset,
        double f,
        double lowerBound,
        double upperBound
    );

    /**
     * @brief Differential Evolution optimizer
     * 
     * @tparam T1 Type of the optimization problem
     * @param problem Optimization problem object
     * @param popSize Population size
     * @param f Differential weight
     * @param cr Crossover rate
     * @param maxGenerations Maximum generations
     * @param seed Random seed
     * @return std::vector<double> Best solution found
     */
    template<typename Problem>
    static std::vector<double> optimize(
        Problem& problem,
        size_t popSize,
        double f,
        double cr,
        size_t maxGenerations,
        unsigned long seed
    );
};

#include "Optimizer/DifferentialEvolution/DifferentialEvolution.tpp"


#endif
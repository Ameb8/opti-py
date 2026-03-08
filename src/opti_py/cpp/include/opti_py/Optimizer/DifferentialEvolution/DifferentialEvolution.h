#ifndef DIFFERENTIAL_EVOLUTION_H
#define DIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <cstddef>
#include "External/mt.h"
#include "Optimizer/Evaluable.h"
#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"
#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"


class DifferentialEvolution {
public:
    DifferentialEvolution() = default;
    ~DifferentialEvolution() = default;


    static std::unique_ptr<Mutation> createMutation(
        const std::string& name
    );


    static std::unique_ptr<Crossover> createCrossover(
        const std::string& name
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
     * @brief Differential Evolution optimizer
     * 
     * @tparam Evaluable optimization problem
     * @param problem Optimization problem object
     * @param popSize Population size
     * @param f Differential weight
     * @param cr Crossover rate
     * @param maxGenerations Maximum generations
     * @param seed Random seed
     * @return std::vector<double> Best solution found
     */
    template<Evaluable Problem>
    static std::vector<double> optimize(
        Problem& problem,
        size_t popSize,
        double f,
        double cr,
        size_t maxGenerations,
        unsigned long seed,
        std::string& mutationStrategy,
        std::string& crossoverStrategy
    );
};

#include "Optimizer/DifferentialEvolution/DifferentialEvolution.tpp"


#endif
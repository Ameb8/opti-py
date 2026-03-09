#ifndef DIFFERENTIAL_EVOLUTION_H
#define DIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <cstddef>
#include "External/mt.h"
#include "Optimizer/Evaluable.h"
#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"
#include "Optimizer/DifferentialEvolution/Mutation/AllMutations.h"
#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"
#include "Optimizer/DifferentialEvolution/Crossover/AllCrossovers.h"



class DifferentialEvolution {
public:
    DifferentialEvolution() = default;
    ~DifferentialEvolution() = default;


    static inline std::unique_ptr<Mutation> createMutation(
        const std::string& name
    ) {
        if(name == "rand1")
            return std::make_unique<Rand1>();
        if(name == "rand2")
            return std::make_unique<Rand2>();
        if(name == "best1")
            return std::make_unique<Best1>();
        if(name == "best2")
            return std::make_unique<Best2>();
        if(name == "randToBest1")
            return std::make_unique<RandBest1>(0.8);

        throw std::runtime_error("Unknown mutation strategy: " + name);
    }


    static inline std::unique_ptr<Crossover> createCrossover(
        const std::string& name
    ) {
        if(name == "bin")
            return std::make_unique<BinCrossover>();
        else if(name == "exp")
            return std::make_unique<ExpCrossover>();
        else
            throw std::runtime_error("Unknown crossover strategy");
    }


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
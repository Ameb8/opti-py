/**
 * @file DifferentialEvolution.h
 * @author Alex Buckley
 * @brief Differential Evolution (DE) optimization algorithm implementation.
 * @defgroup DifferentialEvolution Differential Evolution Optimizer
 * @ingroup Optimizers
 *
 * Provides a template-based implementation of the Differential Evolution algorithm
 * for continuous optimization problems. Supports multiple mutation and crossover
 * strategies with OpenMP parallelization for efficient population evaluation.
 */


#ifndef DIFFERENTIAL_EVOLUTION_H
#define DIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <cstddef>
#include <memory>
#include <string>

#include "External/mt.h"
#include "Optimizer/Evaluable.h"
#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"
#include "Optimizer/DifferentialEvolution/Mutation/AllMutations.h"
#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"
#include "Optimizer/DifferentialEvolution/Crossover/AllCrossovers.h"
#include "Optimizer/OptResult.h"


/**
 * @class DifferentialEvolution
 * @ingroup DifferentialEvolution
 * @brief Implements the Differential Evolution metaheuristic algorithm.
 *
 * DifferentialEvolution provides a genetic algorithm-inspired approach to continuous
 * optimization. The algorithm maintains a population of candidate solutions and iteratively
 * evolves them through mutation and crossover operations guided by fitness-based selection.
 *
 * Key features:
 * - Support for multiple mutation strategies (rand1, rand2, best1, best2, randToBest1)
 * - Support for multiple crossover strategies (binomial, exponential)
 * - OpenMP parallelization for population-level operations
 * - Template-based design supporting arbitrary Evaluable problem types
 * - Deterministic seeding for reproducible results
 *
 * @note All methods are static; this class serves as a namespace for DE-related functionality.
 */
class DifferentialEvolution {
public:
   /**
     * @brief Default constructor (implicitly deleted for static utility class).
     */
    DifferentialEvolution() = default;
 
    /**
     * @brief Default destructor.
     */
    ~DifferentialEvolution() = default;
 
    /** @name Factory Methods */
    ///@{
 
    /**
     * @brief Creates a mutation strategy object by name.
     *
     * Supported mutation strategies:
     * - "rand1": DE/rand/1 - Random base with one difference vector
     * - "rand2": DE/rand/2 - Random base with two difference vectors
     * - "best1": DE/best/1 - Best base with one difference vector
     * - "best2": DE/best/2 - Best base with two difference vectors
     * - "randToBest1": DE/rand-to-best/1 - Interpolation between rand and best
     *
     * @param name String identifier for the mutation strategy
     * @return std::unique_ptr<Mutation> Heap-allocated mutation strategy object
     * @throw std::runtime_error if the strategy name is not recognized
     */
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

    /**
     * @brief Creates a crossover strategy object by name.
     *
     * Supported crossover strategies:
     * - "bin": Binomial (uniform) crossover - each parameter inherited independently
     * - "exp": Exponential crossover - contiguous parameter blocks inherited
     *
     * @param name String identifier for the crossover strategy
     * @return std::unique_ptr<Crossover> Heap-allocated crossover strategy object
     * @throw std::runtime_error if the strategy name is not recognized
     */
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


    ///@}
 
    /** @name Core Optimization */
    ///@{
 
    /**
     * @brief Performs binomial crossover between target and mutant vectors.
     *
     * In binomial crossover, each parameter of the trial vector is inherited
     * from the mutant vector with probability @p cr, or from the target vector
     * with probability (1 - @p cr). At least one parameter is always inherited
     * from the mutant to ensure diversity.
     *
     * @param[in,out] target The target (parent) vector to be modified in-place
     * @param[in] mutant The mutant vector used for crossover
     * @param[in] cr Crossover rate in range [0, 1]; controls inheritance probability
     * @param[in,out] mt Mersenne Twister random number generator
     *
     * @post @p target is modified with parameters from @p mutant according to @p cr
     */
    static inline void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    );


    /**
     * @brief Runs the Differential Evolution optimization algorithm.
     *
     * Executes the complete DE optimization loop:
     * 1. Initializes a population using the problem's initial solution generator
     * 2. Evaluates initial population fitness
     * 3. For each generation:
     *    - Applies mutation strategy to generate mutant vectors
     *    - Applies crossover strategy to generate trial vectors
     *    - Evaluates trial vector fitness
     *    - Selects survivors (greedy selection based on fitness)
     *    - Tracks global best solution
     * 4. Returns best solution found and convergence history
     *
     * Thread Safety:
     * - Population evaluation is parallelized with OpenMP
     * - Global best tracking uses critical sections to prevent data races
     * - Each thread maintains thread-local best fitness to minimize synchronization
     *
     * @tparam Problem Must satisfy the Evaluable concept with methods:
     *         - getInitialSolutions(std::vector<std::vector<double>>&)
     *         - evaluateSolution(const std::vector<double>&) -> double
     *         - getLowerBounds() -> double
     *         - getUpperBounds() -> double
     *
     * @param[in,out] problem The optimization problem object
     * @param[in] popSize Population size; must be > 0
     * @param[in] f Differential weight (mutation scaling factor) in range (0, 2]
     * @param[in] cr Crossover rate in range [0, 1]
     * @param[in] maxGenerations Maximum number of generations to evolve
     * @param[in] seed Seed value for random number generation; enables reproducibility
     * @param[in,out] mutationStrategy Name of mutation strategy; defaults to "rand1" if empty
     * @param[in,out] crossoverStrategy Name of crossover strategy; defaults to "bin" if empty
     *
     * @return OptResult struct containing:
     *         - bestSolution: best vector found
     *         - bestFitness: fitness value of best solution
     *         - bestFitnesses: convergence history (best fitness per generation)
     *
     * @note RNG seeding ensures deterministic results: distinct seeds are used for
     *       mutation and crossover at each (generation, individual) pair to prevent
     *       state contamination while maintaining reproducibility.
     *
     * @note Returns empty OptResult if popSize == 0.
     */
    template<Evaluable Problem>
    static OptResult optimize(
        Problem& problem,
        size_t popSize,
        double f,
        double cr,
        size_t maxGenerations,
        unsigned long seed,
        std::string& mutationStrategy,
        std::string& crossoverStrategy
    );
    
    ///@}
};

#include "Optimizer/DifferentialEvolution/DifferentialEvolution.tpp"


#endif
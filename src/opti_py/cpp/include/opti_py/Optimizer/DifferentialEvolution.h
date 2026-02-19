/**
 * @file DifferentialEvolution.h
 * @author Alex Buckley
 * @ingroup Optimizers
 * @brief Header file for the Differential Evolution optimization algorithm.
 */


#ifndef DIFFERENTIAL_EVOLUTION_H
#define DIFFERENTIAL_EVOLUTION_H

#include <memory>

#include "Optimizer/Optimizer.h"
#include "Optimizer/Crossover/Crossover.h"
#include "Optimizer/Mutation/Mutation.h"
#include "ProblemFactory.h"
#include "ExperimentConfig.h"

/**
 * @class DifferentialEvolution
 * @brief Implements a Differential Evolution optimization algorithm.
 *
 * The LocalSearch optimizer explores the neighborhood of the current
 * solution by generating candidate neighbors within a fixed delta.
 * The search continues until a local minimum is reached or the
 * maximum number of iterations is exceeded.
 *
 * This class supports both single-run local search and repeated
 * local search depending on the iteration limit provided.
 */
class DifferentialEvolution : public Optimizer {
private:
    /// Initial population size
    const int popSize;
    const double scale;
    const double crossover;

    std::unique_ptr<Crossover> crossStrat;
    std::unique_ptr<Mutation> mutStrat;

    std::vector<std::vector<double>> initPopulation();

    static std::unique_ptr<Mutation> createMutation(const std::string& name);
    static std::unique_ptr<Crossover> createCrossover(const std::string& name);

public:
    DifferentialEvolution(
        const ExperimentConfig& config,
        double scale,
        double crossoverRate,
        int popSize,
        const std::string& mutationType,
        const std::string& crossoverType)
        : Optimizer(
            std::make_shared<SolutionBuilder>(
                config.dimensions,
                config.lower,
                config.upper,
                config.seed
            ),
            ProblemFactory::create(config.problemType),
            config.maxIterations
        ),
        popSize(popSize),
        scale(scale),
        crossover(crossoverRate),
        mutStrat(DifferentialEvolution::createMutation(mutationType)),
        crossStrat(DifferentialEvolution::createCrossover(crossoverType))
    {}


    /**
     * @brief Executes the Differential Evolution  optimization process.
     *
     * @return The functions execution time.
     */
    std::vector<double> optimize() override;


};

#endif


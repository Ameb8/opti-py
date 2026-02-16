/**
 * @file OptimizerFactory.h
 * @author Alex Buckley
 * @ingroup Optimizers
 * @brief Factory utility for instantiating different optimizer types.
 */


#ifndef OPTIMIZER_FACTORY_H
#define OPTIMIZER_FACTORY_H

#include "Optimizer/Optimizer.h"
#include "Optimizer/Blind.h"
#include "Optimizer/LocalSearch.h"
#include "Optimizer/DifferentialEvolution.h"

#include "Optimizer/Mutation/AllMutations.h"
#include "Optimizer/Crossover/AllCrossovers.h"

#include <memory>
#include <string>


/**
 * @class OptimizerFactory
 * @brief Factory class for creating optimizer instances.
 *
 * The OptimizerFactory encapsulates the logic for selecting and
 * constructing the appropriate optimization algorithm based on
 * experimental configuration parameters.
 */
class OptimizerFactory {
private:
    static std::unique_ptr<DifferentialEvolution> createDefaultDE(
        Problem& problem,
        SolutionBuilder& builder,
        int maxGenerations,
        int popSize,
        double scale,
        double crossoverRate
    ) {
        // Choose fixed strategies here
        auto mutation = std::make_unique<Rand1>();
        auto crossover = std::make_unique<BinCrossover>();

        return std::make_unique<DifferentialEvolution>(
            builder,
            problem,
            maxGenerations,
            popSize,
            scale,
            crossoverRate,
            std::move(crossover),
            std::move(mutation)
        );
    }

public:

    /**
     * @brief Initializes an optimizer based on configuration settings.
     *
     * @param problem Reference to the optimization problem.
     * @param config Experiment configuration specifying optimizer type and parameters.
     * @param builder Reference to the solution builder.
     *
     * @return A unique pointer to the initialized Optimizer instance,
     *         or nullptr if the optimizer type is unsupported.
     */
    static std::unique_ptr<Optimizer> initOptimizer(Problem& problem, ExperimentConfig& config, SolutionBuilder& builder) {
        if(config.optimizer == "blind")
            return std::make_unique<Blind>(builder, problem, config.maxIterations);
        else if(config.optimizer == "local")
            return std::make_unique<LocalSearch>(builder, problem, 1, config.neighborDelta, config.numNeighbors);
        else if(config.optimizer == "repeated local")
            return std::make_unique<LocalSearch>(builder, problem, config.maxIterations, config.neighborDelta, config.numNeighbors);
        else if(config.optimizer == "DE")
            return OptimizerFactory::createDefaultDE(problem, builder, config.maxIterations, config.popSize, config.scale, config.cr);
        
        return nullptr;
    }


};

#endif
/**
 * @file Blind.h
 * @author Alex Buckley
 * @ingroup Optimizers
 * @brief Header file for the Blind (Random Walk) optimization algorithm.
 */


#ifndef BLIND_H
#define BLIND_H

#include "Optimizer/Optimizer.h"


/**
 * @class Blind
 * @brief Implements a blind (random walk) optimization algorithm.
 *
 * The Blind optimizer repeatedly samples random solutions from the
 * solution space without using neighborhood information or gradient
 * guidance. It keeps track of the best solution found across all
 * iterations.
 *
 * This algorithm serves as a baseline for comparison against more
 * informed local search techniques.
 */
class Blind : public Optimizer {
public:
    /**
     * @brief Constructs a Blind optimizer.
     *
     * @param solutionBuilder Reference to the solution generator.
     * @param problem Reference to the optimization problem.
     * @param maxIterations Maximum number of iterations to perform.
     */

    Blind(std::shared_ptr<SolutionBuilder> solutionBuilder, std::shared_ptr<Problem> problem, int maxIterations)
        : Optimizer(solutionBuilder, problem, maxIterations)
    {
        bestFitnesses.resize(maxIterations);
        bestSolution.resize(solutionBuilder->getDimensions());
        solutions.resize(maxIterations);
    }

    /**
     * @brief Executes the blind optimization process.
     *
     * @return Execution time of the algorithm.
     */
    std::vector<double> optimize() override;
};

#endif
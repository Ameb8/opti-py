/**
 * @file LocalSearch.h
 * @author Alex Buckley
 * @ingroup Optimizers
 * @brief Header file for the Local Search optimization algorithm.
 */


#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "Optimizer/Optimizer.h"

/**
 * @class LocalSearch
 * @brief Implements a local search optimization algorithm.
 *
 * The LocalSearch optimizer explores the neighborhood of the current
 * solution by generating candidate neighbors within a fixed delta.
 * The search continues until a local minimum is reached or the
 * maximum number of iterations is exceeded.
 *
 * This class supports both single-run local search and repeated
 * local search depending on the iteration limit provided.
 */
class LocalSearch : public Optimizer {
private:
    /// Maximum perturbation applied when generating neighboring solutions
    const double delta;

    /// Number of neighboring solutions evaluated per iteration
    const int numNeighbors;

    /**
     * @brief Performs a single local search run until convergence.
     *
     * Iteratively explores neighboring solutions and updates the
     * current solution if an improvement is found.
     */
    void localSearch();

public:
    /**
     * @brief Constructs a LocalSearch optimizer.
     *
     * @param solutionBuilder Reference to the solution generator.
     * @param problem Reference to the optimization problem.
     * @param maxIterations Maximum number of local search iterations.
     * @param delta Neighborhood radius for neighbor generation.
     * @param numNeighbors Number of neighbors sampled per iteration.
     */
    LocalSearch(
        std::shared_ptr<SolutionBuilder> solutionBuilder, 
        std::shared_ptr<Problem> problem, 
        int maxIterations, 
        double delta, 
        int numNeighbors)
        : Optimizer(solutionBuilder, problem, maxIterations),
          delta(delta),
          numNeighbors(numNeighbors)
    {}

    /**
     * @brief Executes the local search optimization process.
     *
     * @return The functions execution time.
     */
    std::vector<double> optimize() override;
};

#endif


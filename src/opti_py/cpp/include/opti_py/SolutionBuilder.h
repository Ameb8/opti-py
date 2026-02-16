/**
 * @file SolutionBuilder.h
 * @ingroup Utilities
 * @brief Utility class for generating and manipulating candidate solutions.
 * @author Alex Buckley
 */


#ifndef SOLUTION_BUILDER_H
#define SOLUTION_BUILDER_H

#include <vector>

#include "Problem/Problem.h"
#include "External/mt.h"


/**
 * @class SolutionBuilder
 * @brief Responsible for creating random solutions and neighborhood samples.
 *
 * This class encapsulates the logic for generating initial random positions 
 * within the search space and perturbing existing solutions to find neighbors.
 * It utilizes the Mersenne Twister algorithm for high-quality random number generation.
 */
class SolutionBuilder { 
private:
    const int dimensions;   ///< Dimensionality of the problem space
    const int lower;        ///< Lower boundary for coordinate values
    const int upper;        ///< Upper boundary for coordinate values
    MersenneTwister mt;     ///< Random number generator instance


public:
    /**
     * @brief Constructs a SolutionBuilder with specific space constraints.
     * @param dimensions Number of variables in the solution vector.
     * @param lower Minimum value for any given dimension.
     * @param upper Maximum value for any given dimension.
     * @param seed Value used to initialize the Mersenne Twister generator.
     */
    SolutionBuilder(int dimensions, int lower, int upper, int seed)
        : dimensions(dimensions),
          lower(lower),
          upper(upper)
    {
        mt.init_genrand(seed);
    }

    /**
     * @brief Generates a single random solution vector within bounds.
     * @return A vector of size @ref dimensions with values in range [lower, upper].
     */
    std::vector<double> getRand();

    /**
     * @brief Generates a set of neighboring solutions around a central point.
     *
     * Used primarily by Local Search algorithms to explore the immediate vicinity 
     * of the current best candidate.
     *
     * @param center The original solution vector to perturb.
     * @param numNeighbors Number of neighbor vectors to generate.
     * @param maxDelta The maximum step size allowed for perturbation in any dimension.
     * @return A collection of neighboring solution vectors.
     */
    std::vector<std::vector<double>> getNeighbors(
        const std::vector<double>& center,
        int numNeighbors,
        double maxDelta
    );

    /** @brief Returns the dimensionality of the solution space. */
    double getDimensions() { return dimensions; }


    /**
     * @brief Generates a subset of unique solutions from given population.
     *
     * @param population Size population from which subset is selected.
     * @param subsetSize Number of solutions to include in returned subset.
     * @return A collection of unique indices from initial population.
     */
    std::vector<int> getSubset(
        int populationSize,
        int subsetSize,
        int source
    );

    std::vector<double> getDifference(
        const std::vector<double> vec1,
        const std::vector<double> vec2
    );

    /**
     * @brief Ensures a coordinate stays within the defined [lower, upper] bounds.
     * @param value The value to check.
     * @return The clamped value.
     */
    double checkBounds(double value);

    double randNum() { return mt.genrand_real1(); }
    int randNum(int x, int y) { return x + mt.genrand_int32() % (y - x); }
};

#endif

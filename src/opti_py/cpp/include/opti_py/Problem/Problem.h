/**
 * @file Problem.h
 * @author Alex Buckley
 * @defgroup Benchmarks Optimization Problems
 * @ingroup Benchmarks
 * @brief Base class and group definition for all optimization benchmark functions.
 */

#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>
#include <omp.h>

#include "External/mt.h"

/**
 * @class Problem
 * @brief Abstract base class for all optimization benchmark problems.
 *
 * Provides a common interface for evaluating fitness and retrieving 
 * search space boundaries.
 */
class Problem {
protected:
    const double lowerBound;  ///< Lower bound of the search space
    const double upperBound;  ///< Upper bound of the search space
    const std::string name;   ///< Name of the benchmark function
    size_t dimensions;
    unsigned long seed = 0;

public:
    /**
     * @brief Constructs a Problem instance.
     * * @param lb Lower bound for the solution space.
     * @param ub Upper bound for the solution space.
     * @param n  Name of the optimization function.
     */
    Problem(double lb, double ub, const std::string_view n)
        : lowerBound(lb), upperBound(ub), name(n) {}

    /**
     * @brief Virtual destructor for safe polymorphic cleanup.
     */
    virtual ~Problem() = default;

    /**
     * @brief Evaluates the fitness of a candidate solution.
     * * This is a pure virtual function that must be implemented by specific 
     * benchmark functions (e.g., Ackley, Rosenbrock).
     * * @param x The solution vector to evaluate.
     * @return The scalar fitness value (cost).
     */
    virtual double evaluate(const std::vector<double>& x) const = 0;

    double evaluateSolution(const std::vector<double>& solution) {
        return evaluate(solution);
    }

    void getInitialSolutions(std::vector<std::vector<double>>& population) {
        MersenneTwister mt;
        mt.init_genrand(seed);

        for(int i = 0; i < population.size(); i++) {
            population[i].resize(dimensions);
            for(int j = 0; j < population[i].size(); j++) {
                population[i][j] = lowerBound + (upperBound - lowerBound) * mt.genrand_real1();
            }
        }
    }

    /** @name Accessors */
    ///@{

    /** @return The lower boundary of the search space. */
    double getLowerBounds() const { return lowerBound; }

    /** @return The upper boundary of the search space. */
    double getUpperBounds() const { return upperBound; }

    /** @return The name of the benchmark function. */
    const std::string getName() const { return name; }

    unsigned long getSeed() const {
        return seed;
    }

    void setSeed(unsigned long s) {
        seed = s;
    }

    void setDimensions(size_t dims) {
        dimensions = dims;
    }


    ///@}
};

#endif // PROBLEM_H

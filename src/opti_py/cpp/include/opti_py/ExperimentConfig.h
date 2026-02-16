/**
 * @file ExperimentConfig.h
 * @author Alex Buckley
 * @ingroup Core
 * @brief Defines the configuration structure for optimization experiments.
 */


#ifndef EXPERIMENT_CONFIG_H
#define EXPERIMENT_CONFIG_H

#include <string>
#include <map>


/**
 * @struct ExperimentConfig
 * @brief Container for all parameters required to execute a benchmark run.
 * * This structure is typically populated by ExperimentRunner from JSON
 * and passed to factories and runners to initialize the experiment state.
 */
struct ExperimentConfig {
    int problemType;            ///< ID of the benchmark problem to instantiate
    int dimensions;             ///< Number of dimensions for the problem space
    double lower;               ///< Global lower bound
    double upper;               ///< Global upper bound
    int seed;                   ///< Pseudo-random number generator seed for reproducibility
    int maxIterations;          ///< Termination criteria: maximum evaluation cycles
 
    ExperimentConfig(
        int problemType,
        int dimensions,
        double lower,
        double upper,
        int seed = 108664,
        int maxIterations = 30
    )
        : problemType(problemType),
          dimensions(dimensions),
          lower(lower),
          upper(upper),
          seed(seed),
          maxIterations(maxIterations)
    {}


};


#endif
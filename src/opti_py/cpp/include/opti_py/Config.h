/**
 * @file Config.h
 * @author Alex Buckley
 * @ingroup Core
 * @brief Defines the configuration structure for optimization experiments.
 */


#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>


/**
 * @struct ExperimentConfig
 * @brief Container for all parameters required to execute a benchmark run.
 * * This structure is typically populated by ExperimentRunner from JSON
 * and passed to factories and runners to initialize the experiment state.
 */
typedef struct {
    std::string experimentName; ///< Identifier for the specific experiment run
    int problemType;            ///< ID of the benchmark problem to instantiate
    int dimensions;             ///< Number of dimensions for the problem space
    double lower;               ///< Global lower bound override
    double upper;               ///< Global upper bound override
    int seed;                   ///< Random number generator seed for reproducibility
    std::string optimizer;      ///< Name/Type of the optimizer algorithm to use
    int maxIterations;          ///< Termination criteria: maximum evaluation cycles
    double neighborDelta;       ///< Step size for neighborhood exploration (Local Search only)
    int numNeighbors;           ///< Number of neighbors to sample per iteration (Local Search only)
    
    // DE Config
    std::optional<std::string> diffEvolution; 
    std::optional<int> popSize;
    std::optional<double> scale;
    std::optional<double> cr;
} ExperimentConfig;

inline ExperimentConfig makeConfigFromArgs(const std::map<std::string, std::string>& args) {
    ExperimentConfig cfg;

    cfg.experimentName = args.find("--experiment_name");
    cfg.problemType = std::stoi(args.at("--problem_type"));
    cfg.dimensions = std::stoi(args.at("--dimensions"));
    cfg.lower = std::stod(args.at("--lower_bound"));
    cfg.upper = std::stod(args.at("--upper_bound"));
    cfg.seed = std::stoi(args.at("--seed"));

    cfg.optimizer = args.at("--optimizer_type");
    cfg.maxIterations = std::stoi(args.at("--iterations"));
    cfg.neighborDelta = std::stod(args.at("--delta"));
    cfg.numNeighbors = std::stoi(args.at("--num_neighbors"));

    return cfg;
}

#endif
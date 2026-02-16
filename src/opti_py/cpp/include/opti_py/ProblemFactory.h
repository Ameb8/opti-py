/**
 * @file ProblemFactory.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Factory for instantiating benchmark problems by ID.
 */

#ifndef PROBLEM_FACTORY_H
#define PROBLEM_FACTORY_H

#include "Problem/Problem.h"
#include <array>
#include <memory>
#include <stdexcept>

/**
 * @class ProblemFactory
 * @brief Utility to create problem instances dynamically.
 */
class ProblemFactory {
public:
    /**
     * @brief Factory method to instantiate a specific benchmark problem.
     * * @param id Integer ID corresponding to the desired problem (e.g., 1 for Ackley, 2 for DeJong).
     * @return std::unique_ptr<Problem> A pointer to the newly created problem instance.
     * @throws std::invalid_argument if the ID does not match a known problem.
     */
    static std::shared_ptr<Problem> create(int id);
};


#endif
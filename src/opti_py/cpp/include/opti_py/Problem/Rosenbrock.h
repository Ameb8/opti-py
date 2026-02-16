/**
 * @file Rosenbrock.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Rosenbrock function.
 */


#ifndef ROSENBROCK_H
#define ROSENBROCK_H

#include "Problem/Problem.h"
#include <vector>


/**
 * @class Rosenbrock
 * @brief Implements the Rosenbrock benchmark function.
 */
class Rosenbrock : public Problem {
private:
    static constexpr double LOWER = -100.0;
    static constexpr double UPPER = 100.0;
    static constexpr std::string_view NAME = "Rosenbrock";

public:
    Rosenbrock() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;
        
        for(size_t i = 0; i < x.size() - 1; i++) {
            // Calculate inner terms
            const double term1 = x[i] * x[i] - x[i+1];
            const double term2 = 1 - x[i];

            // Calculate value for summation
            sum += 100 * term1 * term1 + term2 * term2;
        }

        return sum;
    }
};

#endif

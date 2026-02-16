/**
 * @file Rastrigin.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Ratrigin function.
 */


#ifndef RASTRIGIN_H
#define RASTRIGIN_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>
#include <numbers>


/**
 * @class Rastrigin
 * @brief Implements the Rastrigin benchmark function.
 */
class Rastrigin : public Problem {
private:
    static constexpr double LOWER = -30.0;
    static constexpr double UPPER = 30.0;
    static constexpr std::string_view NAME = "Rastrigin";

public:
    Rastrigin() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;
        
        for(double xi : x) // Calculate summation
            sum += xi * xi - 10 * std::cos(2 * std::numbers::pi * xi);

        return 10 * x.size() + sum;
    }
};

#endif

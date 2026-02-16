/**
 * @file AckleyTwo.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Ackley 2 function.
 */


#ifndef ACKLEY_TWO_H
#define ACKLEY_TWO_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>
#include <numbers>


/**
 * @class AckleyTwo
 * @brief Implements the Ackley 2 benchmark function.
 */
class AckleyTwo : public Problem {
private:
    static constexpr double LOWER = -32.0;
    static constexpr double UPPER = 32.0;
    static constexpr std::string_view NAME = "AckleyTwo";

public:
    AckleyTwo() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;

        for(size_t i = 0; i < x.size() - 1; i++) {
            double sqrtTerm = std::sqrt((x[i] * x[i] + x[i+1] * x[i+1]) / 2);
            double term3 = -20 / (std::exp(0.2 * sqrtTerm));
            double term4 = -1 * std::exp(0.5 * std::cos(2 * std::numbers::pi * x[i]) + std::cos(2 * std::numbers::pi * x[i+1]));

            sum += 20 + std::exp(1.0) + term3 + term4;
        }

        return sum;
    }
};

#endif

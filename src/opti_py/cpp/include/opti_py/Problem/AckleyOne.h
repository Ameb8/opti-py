/**
 * @file AckleyOne.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Ackley 1 function.
 */


#ifndef ACKLEY_ONE_H
#define ACKLEY_ONE_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>


/**
 * @class AckleyOne
 * @brief Implements the Ackley 1 benchmark function.
 */
class AckleyOne : public Problem {
private:
    static constexpr double LOWER = -32.0;
    static constexpr double UPPER = 32.0;
    static constexpr std::string_view NAME = "AckleyOne";

public:
    AckleyOne() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;

        for(size_t i = 0; i < x.size() - 1; i++) {
            double term1 = std::exp(-0.2) * std::sqrt(x[i] * x[i] + x[i+1] * x[i+1]);
            double term2 = 3 * (std::cos(2 * x[i]) + std::sin(2 * x[i+1]));
            sum += term1 + term2;
        }

        return sum;
    }
};

#endif

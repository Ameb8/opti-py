/**
 * @file StretchedV.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Stretched V function.
 */


#ifndef STRETCHED_V_H
#define STRETCHED_V_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>


/**
 * @class StretchedV
 * @brief Implements the StretchedV benchmark function.
 */
class StretchedV : public Problem {
private:
    static constexpr double LOWER = -30.0;
    static constexpr double UPPER = 30.0;
    static constexpr std::string_view NAME = "StretchedV";

public:
    StretchedV() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;

        for(size_t i = 0; i < x.size() - 1; i++) {
            double sqrSum = x[i] * x[i] + x[i+1] * x[i+1];
            double factor1 = std::sqrt(std::sqrt(sqrSum));
            double factor2 = std::sin(50 * std::pow(sqrSum, 0.1));

            sum += factor1 * factor2 * factor2 + 1;
        }

        return sum;
    }
};

#endif

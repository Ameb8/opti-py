/**
 * @file Griewangk.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Griewangk function.
 */

#ifndef GRIEWANGK_H
#define GRIEWANGK_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>


/**
 * @class Griewangk
 * @brief Implements the Griewangk benchmark function.
 */
class Griewangk : public Problem {
private:
    static constexpr double LOWER = -500.0;
    static constexpr double UPPER = 500.0;
    static constexpr std::string_view NAME = "Griewangk";

public:
    Griewangk() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;
        double prod = 1.0;

        for(size_t i = 0; i < x.size(); i++) {
            sum += x[i] * x[i];
            prod *= std::cos(x[i] / std::sqrt(i + 1));
        }

        return 1.0 + sum / 4000.0 - prod;
    }
};

#endif

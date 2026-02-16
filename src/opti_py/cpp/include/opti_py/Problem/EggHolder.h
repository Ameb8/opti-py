/**
 * @file EggHolder.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Egg Holder function.
 */


#ifndef EGG_HOLDER_H
#define EGG_HOLDER_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>


/**
 * @class EggHolder
 * @brief Implements the Egg Holder benchmark function.
 */
class EggHolder : public Problem {
private:
    static constexpr double LOWER = -500.0;
    static constexpr double UPPER = 500.0;
    static constexpr std::string_view NAME = "EggHolder";

    inline double sinSqrtAbs(double n) const {
        return std::sin(std::sqrt(std::abs(n)));
    }

public:
    EggHolder() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;

        for(size_t i = 0; i < x.size() - 1; i++) {
            double term1 = -x[i] * sinSqrtAbs(x[i] - x[i+1] - 47);
            double term2 = -(x[i+1] + 47) * sinSqrtAbs(x[i+1] + 47 + x[i] / 2);

            sum += term1 + term2;
        }

        return sum;
    }
};

#endif

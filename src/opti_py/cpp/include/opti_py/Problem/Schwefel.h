/**
 * @file Schwefel.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Schwefel function.
 */


#ifndef SCHWEFEL_H
#define SCHWEFEL_H

#include "Problem/Problem.h"
#include <vector>


/**
 * @class Schwefel
 * @brief Implements the Schwefel benchmark function.
 */
class Schwefel : public Problem {
private:
    static constexpr double LOWER = -512.0;
    static constexpr double UPPER = 512.0;
    static constexpr std::string_view NAME = "Schwefel";

public:
    Schwefel() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;
        
        for(double xi : x)
            sum += xi * std::sin(std::sqrt(std::abs(xi)));
        
        return 418.9829 * x.size() - sum;
    }
};

#endif

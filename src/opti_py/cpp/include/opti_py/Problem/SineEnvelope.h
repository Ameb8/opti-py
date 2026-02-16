/**
 * @file SineEnvelope.h
 * @author Alex Buckley
 * @ingroup Benchmarks
 * @brief Implementation of the Sine Envelope function.
 */


#ifndef SINE_ENVELOPE_H
#define SINE_ENVELOPE_H

#include "Problem/Problem.h"
#include <vector>
#include <cmath>


/**
 * @class SineEnvelope
 * @brief Implements the Sine Envelope benchmark function.
 */
class SineEnvelope : public Problem {
private:
    static constexpr double LOWER = -30.0;
    static constexpr double UPPER = 30.0;
    static constexpr std::string_view NAME = "SineEnvelope";

public:
    SineEnvelope() : Problem(LOWER, UPPER, NAME) {}

    double evaluate(const std::vector<double>& x) const override {
        double sum = 0.0;

        for(size_t i = 0; i < x.size() - 1; i++) {
            double sqrSum = x[i] * x[i] + x[i+1] * x[i+1];
            double numerator = std::sin(sqrSum - 0.5) * std::sin(sqrSum - 0.5);
            double denom = (1 + 0.001 * sqrSum) * (1 + 0.001 * sqrSum);
            sum += numerator / denom + 0.5;
        }

        return sum * -1;
    }
};

#endif

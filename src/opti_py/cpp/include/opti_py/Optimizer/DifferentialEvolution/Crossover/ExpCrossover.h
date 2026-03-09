#ifndef EXP_CROSSOVER_H
#define EXP_CROSSOVER_H


#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"

#include <cstddef>


class ExpCrossover : public Crossover {
public:
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) override {
        size_t jrand = mt.genrand_int32() % target.size();
        size_t left = 0;

        do {
            size_t idx = (jrand + left) % target.size();
            target[idx] = mutant[idx];
            left++;
        } while (mt.genrand_real1() < cr && left < target.size());
    }
};


#endif

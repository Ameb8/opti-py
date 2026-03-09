#ifndef BIN_CROSSOVER_H
#define BIN_CROSSOVER_H

#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"

#include <cstddef>


class BinCrossover : public Crossover {
public:
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) override {
        size_t jrand = mt.genrand_int32() % target.size();

        for(size_t i = 0; i < target.size(); i++) {
            if (i == jrand || mt.genrand_real1() < cr)
                target[i] = mutant[i];
        }
    }
};



#endif

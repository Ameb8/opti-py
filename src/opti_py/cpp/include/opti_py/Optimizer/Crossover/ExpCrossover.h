#ifndef EXP_CROSSOVER_H
#define EXP_CROSSOVER_H


#include "Optimizer/Crossover/Crossover.h"


class ExpCrossover : public Crossover {
public:
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double CR,
        SolutionBuilder& builder
    ) override {
        int start = builder.randNum(0, target.size());
        int L = 0;

        do {
            int idx = (start + L) % target.size();
            target[idx] = mutant[idx];
            L++;
        } while (builder.randNum() < CR && L < target.size());
    }
};


#endif

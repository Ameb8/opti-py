#ifndef BIN_CROSSOVER_H
#define BIN_CROSSOVER_H

#include "Optimizer/Crossover/Crossover.h"



class BinCrossover : public Crossover {
public:
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double CR,
        SolutionBuilder& builder
    ) override {
        int jrand = builder.randNum(0, target.size());

        for(int i = 0; i < target.size(); i++) {
            if (i == jrand || builder.randNum() < CR)
                target[i] = mutant[i];
        }
    }
};


#endif

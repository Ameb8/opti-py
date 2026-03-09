#ifndef OPT_RESULT_H
#define OPT_RESULT_H

#include <vector>

class OptResult {
public:
    std::vector<double> bestSolution;
    std::vector<double> bestFitnesses;
    double bestFitness;
};

#endif
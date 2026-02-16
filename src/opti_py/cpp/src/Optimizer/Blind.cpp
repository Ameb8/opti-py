#include "Optimizer/Blind.h"

#include <limits>


std::vector<double> Blind::optimize() {
    // Iterate population
    for(int i = 0; i < maxIterations; i++) {
        // Get neighbors
        solutions[i] = solutionBuilder->getRand();

        // Set next fitness
        bestFitnesses[i] = problem->evaluate(solutions[i]);

        // Update best fitness
        if(i > 0 && bestFitnesses[i-1] < bestFitnesses[i]) {
            bestFitnesses[i] = bestFitnesses[i - 1];
            solutions[i] = solutions[i - 1];
        }
    }

    return bestFitnesses;
}

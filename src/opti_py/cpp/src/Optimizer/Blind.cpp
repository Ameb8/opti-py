#include "Optimizer/Blind.h"

#include <limits>


double Blind::optimize() {
    // Start timing
    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();

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

    // Return execution time
    return std::chrono::duration<double>(clock::now() - start).count();
}

#include "Optimizer/LocalSearch.h"

#include <limits>

void LocalSearch::localSearch() {
    // Get initial population pseudo-randomly
    std::vector<double> curSolution = solutionBuilder->getRand();
    double curFitness = problem->evaluate(curSolution);
    bool minimaFound = false;

    // Loop until local minima found
    while(!minimaFound) {
        minimaFound = true;

        // Get set of neighbors
        std::vector<std::vector<double>> neighbors = solutionBuilder->getNeighbors(
            curSolution,
            numNeighbors,
            delta
        );

        // Track local minima
        int bestNeighborIdx = -1;
        double bestNeighborFitness = std::numeric_limits<double>::max();

        // Check all neighbors
        for(size_t i = 0; i < neighbors.size(); i++) {
            // Evaluate neighbor's fitness
            double neighborFitness = problem->evaluate(neighbors[i]);

            // Better neighbor found, update stats
            if(neighborFitness < bestNeighborFitness) {
                bestNeighborIdx = i;
                bestNeighborFitness = neighborFitness;
            }
        }   

        // Compare best neighbor to center fitness
        if(bestNeighborFitness < curFitness) {
            minimaFound = false;
            curSolution = neighbors[bestNeighborIdx];
            curFitness = bestNeighborFitness;
        } 

        // Append best found fitness to results
        if(!bestFitnesses.size() || bestFitnesses.back() > curFitness)
            bestFitnesses.push_back(curFitness); // New best found
        else // No new best found
            bestFitnesses.push_back(bestFitnesses.back());
    }
}



std::vector<double> LocalSearch::optimize() {
    for(int i = 0; i < maxIterations; i++)
        localSearch();

    return bestFitnesses;
}
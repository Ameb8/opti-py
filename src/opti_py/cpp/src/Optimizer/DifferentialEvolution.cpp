#include "Optimizer/DifferentialEvolution.h"

#include <limits>
#include <stdexcept>

#include "Optimizer/Mutation/AllMutations.h"
#include "Optimizer/Crossover/AllCrossovers.h"


std::vector<std::vector<double>> DifferentialEvolution::initPopulation() {
    std::vector<std::vector<double>> pop(popSize);

    for(int i = 0; i < popSize; i++) {
        pop[i] = solutionBuilder->getRand();
    }

    return pop;
}

std::unique_ptr<Mutation> DifferentialEvolution::createMutation(
    const std::string& name
) {
    // Create mutation type
    if(name == "rand1") 
        return std::make_unique<Rand1>();
    if(name == "rand2") 
        return std::make_unique<Rand2>();
    if(name == "best1") 
        return std::make_unique<Best1>();
    if(name == "best2") 
        return std::make_unique<Best2>();
    if(name == "randToBest1") 
        return std::make_unique<RandBest1>(0.8);

    // Type not recognized
    throw std::runtime_error("Unknown mutation strategy: " + name);
}


std::unique_ptr<Crossover> DifferentialEvolution::createCrossover(
    const std::string& name
) {
    if(name == "bin")
        return std::make_unique<BinCrossover>();
    else if 
    (name == "exp")
        return std::make_unique<ExpCrossover>();
    else
        throw std::runtime_error("Unknown crossover strategy");
}



std::vector<double> DifferentialEvolution::optimize() {
    // Allocate memory to store best fitness per iteration
    bestFitnesses.resize(maxIterations, std::numeric_limits<double>::max());

    // Randomly initialize population
    std::vector<std::vector<double>> pop = initPopulation();
    
    // Stores initial fitness values
    std::vector<double> fitness(popSize);

    int bestIdx = 0;
    
    // Store initial fitnesses
    for(int i = 0; i < popSize; i++) {
        fitness[i] = problem->evaluate(pop[i]);

        if(fitness[i] < fitness[bestIdx])
            bestIdx = i;
    }

    // Track best vector
    double bestFitness = fitness[bestIdx];
    
    for(int i = 0; i < maxIterations; i++) {
        // Temporarily stores new population
        std::vector<std::vector<double>> genLockPop = pop;
        double genBestIdx = bestIdx;

        // Calculate fitnesses for initial population
        for(int j = 0; j < popSize; j++) {
            // Get mutated vector
            std::vector<double> mutated =
                mutStrat->mutate(pop, j, scale, genLockPop[genBestIdx], *solutionBuilder);
                
            // Create crossover vector
            crossStrat->crossover(pop[j], mutated, crossover, *solutionBuilder);

            // Calculate fitness of original and trial vectors
            double trialFitness = problem->evaluate(pop[j]);

            // Restore old solution
            if(fitness[j] < trialFitness) {
                pop[j] = genLockPop[j];
            } else { // Replace old solution
                fitness[j] = trialFitness;
                
                // Update best fitness
                if(trialFitness < bestFitness) {
                    bestFitness = trialFitness;
                    bestIdx = j;
                    bestSolution = pop[j];
                }
            }
        }

        bestFitnesses[i] = bestFitness;
    }


    return bestFitnesses;
}


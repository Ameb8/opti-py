#include "Optimizer/ParticleSwarm.h"

#include <limits>
#include <stdexcept>

#include "Optimizer/Mutation/AllMutations.h"
#include "Optimizer/Crossover/AllCrossovers.h"


std::vector<std::vector<double>> ParticleSwarm::initPopulation() {
    std::vector<std::vector<double>> pop(popSize);

    for(int i = 0; i < popSize; i++) {
        pop[i] = solutionBuilder->getRand();
    }

    return pop;
}


std::vector<std::vector<double>> ParticleSwarm::initVelocities() {
    // Create vector to hold velocities
    std::vector<std::vector<double>> velocity(popSize);
    
    for(int i = 0; i < popSize; i++)
        velocity[i] = solutionBuilder->getInitialVelocity();

    return velocity;
}


void ParticleSwarm::updatePosition(std::vector<double>& solution, std::vector<double>& velocity) {
    for(int i = 0; i < solution.size(); i++) {
        solution[i] += velocity[i];
        solution[i] = solutionBuilder->checkBounds(solution[i]);
    }
}


void ParticleSwarm::updateVelocity(
    std::vector<double>& velocity,
    const std::vector<double>& pop,
    const std::vector<double>& pBestPos,
    const std::vector<double>& gBestPos
) { 
    for(int i = 0; i < pop.size(); i++) {
        // Generate r values
        double r1 = solutionBuilder->randNum();
        double r2 = solutionBuilder->randNum();

        // Calculate change in velocity 
        double cognitive = c1 * r1 * (pBestPos[i] - pop[i]);
        double social = c2 * r2 * (gBestPos[i] - pop[i]);
        
        // Update velocity
        velocity[i] += cognitive + social;
    }
}


std::vector<double> ParticleSwarm::optimize() {
    // Allocate memory to store best fitness per iteration
    bestFitnesses.resize(maxIterations, std::numeric_limits<double>::max());
    int dims = solutionBuilder->getDimensions();
    
    // Randomly initialize population and velocities
    std::vector<std::vector<double>> pop = initPopulation();
    std::vector<std::vector<double>> velocity = initVelocities();
    
    // Personal best fitnesses and solutions
    std::vector<double> pBestFitness(popSize);
    std::vector<std::vector<double>> pBestSolution(
        popSize,
        std::vector<double>(dims)
    );

    // Global best fitness and solution
    double gBestFitness = std::numeric_limits<double>::max();
    std::vector<double> gBestSolution(dims);
    
    // Get initial fitnesses
    for(int i = 0; i < popSize; i++) {
        // Evaluate initial solution
        pBestFitness[i] = problem->evaluate(pop[i]);
        pBestSolution[i] = pop[i];
        
        // Update global best
        if(pBestFitness[i] < gBestFitness) {
            gBestFitness = pBestFitness[i];
            gBestSolution = pBestSolution[i];
        }
    }

    // Iterate generations
    for(int i = 0; i < maxIterations; i++) {
        // Iterate particles
        for(int j = 0; j < popSize; j++) {
            // Update particle's velocity and position
            updateVelocity(velocity[j], pop[j], pBestSolution[j], gBestSolution);
            updatePosition(pop[j], velocity[j]);

            // Evaluate updated solution
            double fitness = problem->evaluate(pop[j]);

            // Update personal and global bests
            if(fitness < pBestFitness[j]) {
                // Update personal best
                pBestFitness[j] = fitness;
                pBestSolution[j] = pop[j];

                // Update global best
                if(fitness < gBestFitness) {
                    gBestFitness = fitness;
                    gBestSolution = pop[j];
                }
            }

            // Update fitness per iteration
            if(fitness < bestFitnesses[i])
                bestFitnesses[i] = fitness;
        }
    }

    bestSolution = gBestSolution;
    return bestFitnesses;
}

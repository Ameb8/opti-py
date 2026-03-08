#pragma once

#include <limits>
#include <omp.h>

/**
 * @tparam Problem Must satisfy the Evaluable concept
 */
template<Evaluable Problem>
std::vector<double> DE::optimize(
    Problem& problem,
    size_t popSize,
    double f,
    double cr,
    size_t maxGenerations,
    unsigned long seed
) {
    // Ensure population exists
    if(popSize == 0)
        return {};

    // Get initial population
    std::vector<std::vector<double>> population(popSize);
    problem.getInitialSolutions(population);

    // Get bounds for problem
    double lower = problem.getLowerBounds();
    double upper = problem.getUpperBounds();

    // Stores fitness of each solution
    std::vector<double> solutionFitnesses(popSize);

    // Store global best solution
    double globalBestFitness = std::numeric_limits<double>::max();
    size_t globalBestIdx;

    // Calculate permutations and fitness for initial population
    // Parallelize with OpenMP
    #pragma omp parallel 
    {
        // Best solution found per thread
        double threadBestFitness = std::numeric_limits<double>::max();
        size_t threadBestIdx = 0;

        // Calculate permutation and fitness in parallel
        #pragma omp for
        for(size_t i = 0; i < popSize; i++) {
            solutionFitnesses[i] = problem.evaluateSolution(
                population[i]
            );

            // Compare solution to thread best
            if(solutionFitnesses[i] < threadBestFitness) {
                // Update thread best
                threadBestFitness = solutionFitnesses[i];
                threadBestIdx = i;
            }
        }

        // Perform reduction to calculate global best solution
        #pragma omp critical {
            // Compare thread best to global best
            if(threadBestFitness < globalBestFitness) {
                // Update global best
                globalBestFitness = threadBestFitness;
                globalBestIdx = threadBestIdx;
            }
        }
    }

    // Store temporary population during generation updates
    std::vector<std::vector<double>> newPopulation;
    std::vector<double> newSolutionFitnesses(popSize);

    // Iterate generations
    for(size_t i = 0; i < maxGenerations; i++) {
        // Copy generation's initial population
        newPopulation = population;

        #pragma omp parallel
        {
            // Track thread best solution
            double threadBestFitness = std::numeric_limits<double>::max();
            size_t threadBestIdx = 0;
            
            // Produce mutated vectors in parallel
            #pragma omp for
            for(size_t j = 0; j < popSize; j++) {
                // Create RNG seeded for each iteration
                MersenneTwister mt;
                mt.init_genrand(seed + i + j); // Ensure determinism

                // Select 3 distinct solutions
                std::vector<size_t> solutionIndexes = getSubset(popSize, 3, j, mt);

                // Generate mutant vector
                std::vector<double> mutant = mutate(
                    population,
                    solutionIndexes,
                    f,
                    lower,
                    upper
                );

                // Generate Trial vector
                crossover(newPopulation[j], mutant, cr, mt);

                // Get trial fitnesses
                newSolutionFitnesses[j] = problem.evaluateSolution(newPopulation[j]);

                // Check if new solution needs reverting to old
                if(solutionFitnesses[j] < newSolutionFitnesses[j]) {
                    // Reset new solution to original 
                    newPopulation[j] = population[j];
                    newSolutionFitnesses[j] = solutionFitnesses[j];
                } else if(newSolutionFitnesses[j] < threadBestFitness) { 
                    // Update best fitness
                    threadBestFitness = newSolutionFitnesses[j];
                    threadBestIdx = j;
                }
            }

            // Update global best with a reduction
            #pragma omp critical
            {
                if(threadBestFitness < globalBestFitness) {
                    globalBestFitness = threadBestFitness;
                    globalBestIdx = threadBestIdx;
                }
            }
        }

        // Update new population
        population = newPopulation;
        solutionFitnesses = newSolutionFitnesses;
    }

    return population[globalBestIdx];
}

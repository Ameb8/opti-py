/**
 * @file DifferentialEvolution.tpp
 * @author Alex Buckley
 * @brief Template implementation of Differential Evolution optimizer.
 * @ingroup DifferentialEvolution
 *
 * This file contains the out-of-line template implementation of the optimize()
 * method. It is included by DifferentialEvolution.h and should not be included directly.
 * 
 * @note All public API documentation is in DifferentialEvolution.h.
 */

#pragma once

#include <limits>
#include <omp.h>
#include <memory>
#include <string>

#include "Optimizer/OptResult.h"

/**
 * @tparam Problem Must satisfy the Evaluable concept
 */
template<Evaluable Problem>
OptResult DifferentialEvolution::optimize(
    Problem& problem,
    size_t popSize,
    double f,
    double cr,
    size_t maxGenerations,
    unsigned long seed,
    std::string& mutationStrategy,
    std::string& crossoverStrategy
) {
    // Ensure population exists
    if(popSize == 0)
        return {};

    // Get initial population
    std::vector<std::vector<double>> population(popSize);
    problem.getInitialSolutions(population);

    // Create results container
    OptResult result;
    result.bestFitnesses.resize(maxGenerations);


    // Get bounds for problem
    double lower = problem.getLowerBounds();
    double upper = problem.getUpperBounds();

    // Assign default mutation and crossover
    if(mutationStrategy == "")
        mutationStrategy = "rand1";
    if(crossoverStrategy == "")
        crossoverStrategy = "bin";

    // Create mutation and crossover objects
    std::unique_ptr<Mutation> mutation = createMutation(mutationStrategy);
    std::unique_ptr<Crossover> crossover = createCrossover(crossoverStrategy);

    // Stores fitness of each solution
    std::vector<double> solutionFitnesses(popSize);

    // Store global best solution
    double globalBestFitness = std::numeric_limits<double>::max();
    size_t globalBestIdx = 0;

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
        #pragma omp critical
        {
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

        // Lock Gen best IDX
        const size_t genBestIdx = globalBestIdx;
        const std::vector<double> genBestSolution = population[genBestIdx];

        #pragma omp parallel
        {
            // Track thread best solution
            double threadBestFitness = std::numeric_limits<double>::max();
            size_t threadBestIdx = 0;
            
            // Produce mutated vectors in parallel
            #pragma omp for
            for(size_t j = 0; j < popSize; j++) {
                // Create RNG seeded for each iteration
                MersenneTwister mtMutate, mtCrossover;

                // Ensure deterministic generation without overlap
                mtMutate.init_genrand(seed + i + j); 
                mtCrossover.init_genrand(
                    seed + i + j + 
                    maxGenerations * popSize // Seed offset
                );
                
                // Generate mutant vector
                std::vector<double> mutant = mutation->mutate(
                    population,
                    j,
                    f,
                    genBestSolution,
                    mtMutate,
                    lower,
                    upper
                );

                // Generate Trial vector
                crossover->crossover(newPopulation[j], mutant, cr, mtCrossover);

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
        
        // Update iteration-best fitness
        result.bestFitnesses[i] = globalBestFitness;

        // Update new population
        population = newPopulation;
        solutionFitnesses = newSolutionFitnesses;
    }

    // Update results
    result.bestSolution = population[globalBestIdx];
    result.bestFitness = globalBestFitness;


    return result;
}

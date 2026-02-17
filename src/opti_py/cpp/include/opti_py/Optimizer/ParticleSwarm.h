/**
 * @file DifferentialEvolution.h
 * @author Alex Buckley
 * @ingroup Optimizers
 * @brief Header file for the Differential Evolution optimization algorithm.
 */


#ifndef PARTICLE_SWARM_H
#define PARTICLE_SWARM_H

#include <memory>

#include "Optimizer/Optimizer.h"
#include "ProblemFactory.h"
#include "ExperimentConfig.h"

/**
 * @class ParticleSwarm
 * @brief Implements the Particle Swarm Optimization algorithm.
 *
 * This optimizer simulates a swarm of particles moving in the search space,
 * updating velocities and positions based on personal and global bests.
 */

class ParticleSwarm : public Optimizer {
private:
    /// Initial population size
    const int popSize;

    const double c1;
    const double c2;

    ParticleSwarm(
        std::shared_ptr<Problem> prob,
        const ExperimentConfig& config,
        int popSize,
        double c1,
        double c2)
        : Optimizer(
              std::make_shared<SolutionBuilder>(
                  config.dimensions,
                  config.lower,
                  config.upper,
                  config.seed
              ),
              prob,
              config.maxIterations
          ),
          popSize(popSize),
          c1(c1),
          c2(c2)
    {}

    std::vector<std::vector<double>> initPopulation();
    std::vector<std::vector<double>> initVelocities();

    void updatePosition(
        std::vector<double>& solution, 
        std::vector<double>& velocity
    );
    
    void updateVelocity(
        std::vector<double>& velocity,
        const std::vector<double>& pop,
        const std::vector<double>& pBestPos,
        const std::vector<double>& gBestPos
    );

public:
    ParticleSwarm(
        const ExperimentConfig& config,
        double c1,
        double c2,
        int popSize)
        : ParticleSwarm(
              ProblemFactory::create(config.problemType),
              config,
              popSize,
              c1,
              c2
          )
    {}




    /**
     * @brief Executes the Differential Evolution  optimization process.
     *
     * @return The functions execution time.
     */
    std::vector<double> optimize() override;


};

#endif


#include "FlowShop/FlowShop.h"

#include <algorithm>
#include <limits>
#include <omp.h>

#include "External/mt.h"
#include "Optimizer/DifferentialEvolution/DifferentialEvolution.h"



FlowShopResult FlowShop::runDE(
    bool blocking, 
    bool optimizeTardiness,
    size_t popSize,
    double f,
    double cr,
    size_t maxGenerations,
    unsigned long seed,
    std::string& mutationStrategy,
    std::string& crossoverStrategy
) {
    // Assign fields needed during DE optimization
    this->blocking = blocking;
    this->optimizeTardiness = optimizeTardiness;
    this->seed = seed;

    // Perform DE optimization
    OptResult result = DifferentialEvolution::optimize(
        *this,
        popSize,
        f,
        cr,
        maxGenerations,
        seed,
        mutationStrategy,
        crossoverStrategy
    );

    // Convert optimized SPV vector to job ordering
    std::vector<size_t> jobOrder(result.bestSolution.size());
    spvToPerm(result.bestSolution, jobOrder);

    // Build and return FlowShopResult object
    return buildResult(
        jobOrder,
        blocking,
        optimizeTardiness,
        result.bestFitnesses
    );
}


FlowShopResult FlowShop::runNEH(bool blocking, bool optimizeTardiness) {
    // Create vector to store total execution time for each job
    std::vector<uint64_t> totalJobTimes(num_jobs_, 0);
    computeRowSums(totalJobTimes);

    // Create vector to store job execution order
    std::vector<size_t> jobOrder(num_jobs_);
    argSortJobs(totalJobTimes, jobOrder, optimizeTardiness);

    // Create vector to store completions times of each job
    std::vector<std::vector<uint64_t>> completionTimes(
        num_jobs_, 
        std::vector<uint64_t>(num_machines_)
    );

    // Calculate completion times for initial job
    std::span<uint64_t> firstJob = getJob(0, jobOrder);
    updateCompletions(firstJob, completionTimes, 0, blocking);

    // Insert remaining jobs
    for(size_t i = 1; i < num_jobs_; i++)
        insertJob(completionTimes, jobOrder, i, blocking, optimizeTardiness);

    // Construct Result object
    FlowShopResult result{
        jobOrder,
        completionTimes[num_jobs_ - 1][num_machines_ - 1],
        completionTimes,
        calculateTardiness(completionTimes, jobOrder),
        result.fitnesses,
    };
    
    return result;
}


FlowShopResult FlowShop::buildResult(
    std::vector<size_t> jobOrder,
    bool blocking,
    bool optimizeTardiness,
    std::vector<double> ranks
) {
    FlowShopResult result;
    result.sequence = jobOrder;

    // Create vector to store completions times of each job
    std::vector<std::vector<uint64_t>> completionTimes(num_jobs_, std::vector<uint64_t>(num_machines_));

    // Calculate each job's completion time
    for(size_t i = 0; i < num_jobs_; i++) {
        std::span<uint64_t> job = getJob(i, jobOrder);
        updateCompletions(job, completionTimes, i, blocking);
    }

    // Assign final optimization results
    result.completionTimes = completionTimes;
    result.makespan = completionTimes[num_jobs_ - 1][num_machines_ - 1];
    result.tardiness = calculateTardiness(completionTimes, jobOrder);
    result.fitnesses = ranks;

    return result;
}


std::vector<double> FlowShop::permToSPV(
    const std::vector<size_t>& permutation
) {
    std::vector<double> spvVec(permutation.size());
    
    // Assign each job its permutation position
    for(size_t i = 0; i < permutation.size(); i++) {
        spvVec[permutation[i]] = static_cast<double>(i);
    }

    // Scale values from position to [-1, 1] SPV range
    for(size_t i = 0; i < permutation.size(); i++) {
        spvVec[i] = -1.0 + 2.0 * spvVec[i] / (permutation.size() - 1);
    }

    return spvVec;
}


void FlowShop::spvToPerm(
    const std::vector<double>& spvVec,
    std::vector<size_t>& permutation
) {
    // Initialize permutations as index
    for(size_t i = 0; i < spvVec.size(); i++)
        permutation[i] = i;

    // Perform argsort by SPV values
    std::sort(
        permutation.begin(),
        permutation.end(),
        [&](size_t a, size_t b) { // Comparator lambda
            if(spvVec[a] != spvVec[b])
                return spvVec[a] > spvVec[b];

            return a < b;
        }
    );
}


void FlowShop::initPopulationVectors(
    std::vector<std::vector<double>>& population,
    bool blocking,
    bool optimizeTardiness,
    unsigned long seed
) {
    // Generate 1 NEH permutation for initial population
    FlowShopResult initSeed = runNEH(blocking, optimizeTardiness);
    std::vector<double> nehSPV = permToSPV(initSeed.sequence);
    population[0] = nehSPV;

    // Fill next K slots with perturbed NEH
    size_t numPerturbed = population.size() / 4; // 25% of population

    // Set scale for perturbation
    double perturbScale = 0.1;
    if(optimizeTardiness)
        perturbScale = 0.02;

    // Generate random permutations in parallel
    #pragma omp parallel for
    for(size_t i = 1; i < population.size(); i++) {
        // Initialize vector size
        population[i].resize(num_jobs_);

        // Create pseudo-random generator each iteration
        MersenneTwister mt;
        mt.init_genrand(seed + i); // Seed deterministically

        // Generate initial SPV vector
        if(i > numPerturbed) { // Generate random SPV vector
            for(size_t j = 0; j < population[i].size(); j++)
                population[i][j] = 2.0 * mt.genrand_real1() - 1.0;
        } else { // Add noise to NEH-generated SPV vector
            for(size_t j = 0; j < num_jobs_; j++) {
                double noise = perturbScale * (2.0 * mt.genrand_real1() - 1.0); // small perturbation
                population[i][j] = std::clamp(nehSPV[j] + noise, -1.0, 1.0);
            }
        }
    }
}


void FlowShop::insertJob(
    std::vector<std::vector<uint64_t>>& completionTimes,
    std::vector<size_t>& jobOrder,
    size_t jobNum,
    bool blocking,
    bool optimizeTardiness
) {
    // Global best makespan or total tardiness to optimize
    uint64_t globalMinRank = std::numeric_limits<uint64_t>::max();
    int64_t bestPos = -1; // Optimal insert position

    // Store completion times after inserting next job optimally
    std::vector<std::vector<uint64_t>> bestCompletion;

    // Get next job to insert
    std::span<uint64_t> nextJob = getJob(jobNum, jobOrder);

    #pragma omp parallel // Parallelize block with OpenMP
    {
        // Create thread-local variables to track best insert found in thread
        uint64_t localMinRank = std::numeric_limits<uint64_t>::max(); // Tracks makespan or tardiness
        int64_t localBestPos = -1;
        std::vector<std::vector<uint64_t>> localBestCompletion;

        // Test all insert positions
        // Parallelized loop with OpenMP
        #pragma omp for schedule(static)
        for(int64_t i = jobNum; i >= 0; i--) {

            // Make private copy of completion times to test next insert position
            std::vector<std::vector<uint64_t>> candidateTimes = completionTimes;

            // Insert at position i
            updateCompletions(nextJob, candidateTimes, i, blocking);

            // Recalculate completion times for all jobs after insert position
            for(size_t j = i + 1; j <= jobNum; j++) {
                // Recalculate next job
                std::span<uint64_t> job = getJob(j - 1, jobOrder);
                updateCompletions(job, candidateTimes, j, blocking);
            }

            uint64_t candidateRank;

            // Assign candidate rank to makespan or total tardiness
            if(!optimizeTardiness) // Derive candidate makespan from completion times
                candidateRank = candidateTimes[jobNum][num_machines_ - 1];
            else // Derive candidate total tardiness from completion times
                candidateRank = calculateTotalTardiness(candidateTimes, jobOrder);


            // Check if this insert position is local thread-best
            if(candidateRank < localMinRank) {
                // Update thread local best
                localMinRank = candidateRank;
                localBestPos = i;
                localBestCompletion = std::move(candidateTimes);
            }
        }

        // Perform reduction to determine global best
        #pragma omp critical
        {
            // Compare thread best makespan to global best
            if(localMinRank< globalMinRank) {
                // Update global best
                globalMinRank = localMinRank;
                bestPos = localBestPos;
                bestCompletion = std::move(localBestCompletion);
            }
        }
    }

    // Update completion times with new job
    completionTimes = std::move(bestCompletion);

    // Update job order of reshuffled jobs
    size_t jobId = jobOrder[jobNum];
    for(size_t j = jobNum; j > bestPos; j--)
        jobOrder[j] = jobOrder[j - 1];

    // Update job order for new job
    jobOrder[bestPos] = jobId;
}


uint64_t FlowShop::evaluateSchedule(
    const std::vector<size_t>& jobOrder,
    bool blocking,
    bool optimizeTardiness
) {
    // Create vector to store completions times of each job
    std::vector<std::vector<uint64_t>> completionTimes(num_jobs_, std::vector<uint64_t>(num_machines_));

    // Calculate each job's completion time
    for(size_t i = 0; i < num_jobs_; i++) {
        std::span<uint64_t> job = getJob(i, jobOrder);
        updateCompletions(job, completionTimes, i, blocking);
    }

    if(optimizeTardiness)
        return calculateTotalTardiness(completionTimes, jobOrder);

    return completionTimes[num_jobs_ - 1][num_machines_ - 1];
}


uint64_t FlowShop::calculateTotalTardiness(
    const std::vector<std::vector<uint64_t>>& completionTimes,
    const std::vector<size_t>& jobOrder
) {
    uint64_t totalTardiness = 0;

    // Iterate through jobs
    for(size_t i = 0; i < num_jobs_; i++) {
        // Get job completion time
        uint64_t completeTime = completionTimes[i][num_machines_ - 1];

        // Calculate job tardiness
        totalTardiness += completeTime - std::min(
            due_dates_[jobOrder[i]],
            completeTime
        );
    }

    return totalTardiness;
}

std::vector<uint64_t> FlowShop::calculateTardiness(
    const std::vector<std::vector<uint64_t>>& completionTimes,
    const std::vector<size_t>& jobOrder
) {
    std::vector<uint64_t> tardiness(num_jobs_);

    // Iterate through jobs
    for(size_t i = 0; i < num_jobs_; i++) {
        // Get job completion time
        uint64_t completeTime = completionTimes[i][num_machines_ - 1];

        // Calculate job tardiness
        tardiness[i] = completeTime - std::min(
            due_dates_[jobOrder[i]],
            completeTime
        );
    }

    return tardiness;
}


void FlowShop::updateCompletions(
    const std::span<uint64_t> jobTimes,
    std::vector<std::vector<uint64_t>>& completionTimes,
    size_t insertRow,
    bool blocking
) {
    if(insertRow == 0) { // Handle first row insert
        completionTimes[0][0] = jobTimes[0];

        for(size_t i = 1; i < num_machines_; i++)
            completionTimes[0][i] = completionTimes[0][i-1] + jobTimes[i];
    } else { // Insert trailing rows
        // Calculate first machine time
        completionTimes[insertRow][0] = completionTimes[insertRow - 1][0] + jobTimes[0];

        for(size_t i = 1; i < num_machines_; i++) {
            // Ensure machine is free and job finished on last machine
            completionTimes[insertRow][i] = jobTimes[i] + std::max(
                completionTimes[insertRow - 1][i],
                completionTimes[insertRow][i-1]
            );
        }
        
        if(blocking) { // Propagate blocking for job
            for(int64_t i = num_machines_ - 2; i >= 0; i--) {
                // Increase completion time to wait until next machine is free
                completionTimes[insertRow][i] = std::max(
                    completionTimes[insertRow][i],
                    completionTimes[insertRow - 1][i+1]
                );
            }
        }

    }
}


// Computes sum of time across machines for each job
void FlowShop::computeRowSums(std::vector<uint64_t>& totalJobTimes) {
    #pragma omp parallel for schedule(static)
    for(size_t i = 0; i < num_jobs_; ++i) {
        uint64_t sum = 0; // Stores total exec time for job
        size_t row_start = i * num_machines_; // Start index of row
    
        // Sum job time cross machines
        for(size_t j = 0; j < num_machines_; ++j)
            sum += jobs_times_[row_start + j];
    
        totalJobTimes[i] = sum;
    }
}


// Performs argsort of total job times
// Initializes jobOrder with job ID in descending order by total time
void FlowShop::argSortJobs(
    std::vector<uint64_t>& totalJobTimes,
    std::vector<size_t>& jobOrder,
    bool sortDueDates
) {
    // Initialize index array
    #pragma omp parallel for
    for(size_t i = 0; i < jobOrder.size(); ++i)
        jobOrder[i] = i;

    if(sortDueDates){
        std::sort(
            jobOrder.begin(),
            jobOrder.end(),
            [this](size_t a, size_t b) {
                return due_dates_[a] > due_dates_[b];
            }
        );
    } else {
        std::sort(
            jobOrder.begin(),
            jobOrder.end(),
            [&totalJobTimes](size_t a, size_t b) {
                return totalJobTimes[a] > totalJobTimes[b];
            }
        );
    }
}


std::span<uint64_t> FlowShop::getJob(
    size_t jobNum,
    const std::vector<size_t>& jobOrder
) {
    return { jobs_times_.data() + jobOrder[jobNum] * num_machines_, num_machines_ };
}

// <Evaluate Problem> template implementation

void FlowShop::getInitialSolutions(std::vector<std::vector<double>>& population) {
    initPopulationVectors(
        population,
        blocking,
        optimizeTardiness,
        seed
    );
}

double FlowShop::evaluateSolution(const std::vector<double>& solution) {
    // Convert SPV vector to job ordering
    std::vector<size_t> permutation(solution.size());
    spvToPerm(solution, permutation);

    // Evaluate job ordering
    return static_cast<double>(evaluateSchedule(
        permutation,
        blocking,
        optimizeTardiness
    ));
}
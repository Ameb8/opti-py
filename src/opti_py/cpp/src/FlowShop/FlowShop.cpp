#include "FlowShop/FlowShop.h"

#include <algorithm>
#include <limits>
#include <omp.h>

#include "External/mt.h"


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
        calculateTardiness(completionTimes, jobOrder)
    };
    
    return result;
}


std::vector<double> FlowShop::permToSPV(
    const std::vector<size_t>& permutation
) {
    std::vector<double> spvVec(permutation.size());
    
    // Assign each job its permutation position
    for(size_t i = 0; i < permutation.size(); i++)
        spvVec[permutation[pos]] = static_cast<double>(pos);

    // Scale values from position to [-1, 1] SPV range
    for(size_t i = 0; i < permutation.size(); i++)
        spvVec[i] = -1.0 + 2.0 * spvVec[i] / (n - 1);

    return spvVec;
}

void spvToPerm(
    const std::vector<double>& spvVec,
    std::vector<size_t>& permutation
) {
    // Initialize permutations as index
    for(size_t i = 0; i < n; i++)
        permutation[i] = i;

    // Perform argsort by SPV values
    std::sort(
        permutation.begin(),
        permutation.end(),
        [&](size_t a, size_t b) { // Comparator lambda
            return spvVec[a] > spvVec[b];
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
    population[0] = permToSPV(initSeed.sequence);

    // Generate random permutations in parallel
    #pragma omp parallel for
    for(size_t i = 1; i < population.size(); i++) {
        // Create pseudo-random generator each iteration
        MersenneTwister mt;
        mt.init_genrand(seed + i); // Seed deterministically

        // Create randomized SPV vector
        for(size_t j = 0; j < population[i].size(); j++)
            population[i][j] = 2.0 * mt.random() - 1.0;
    }
}


std::vector<size_t> FlowShop::getSubset(
    size_t populationSize,
    size_t subsetSize,
    size_t source,
    MersenneTwister& mt
) {
    std::vector<size_t> indices(populationSize - 1);
    size_t idx = 0;

    // Prepare vector of all valid indices except source
    for(size_t i = 0; i < populationSize; i++) {
        if (i != source) {
            indices[idx] = i;
            ++idx;
        }
    }

    // Partial Fisher-Yates shuffle to select subsetSize random indices
    for(size_t i = 0; i < subsetSize; i++) {
        int j = i + (mt.genrand_int32() % (indices.size() - i));
        std::swap(indices[i], indices[j]);
    }

    // Return only the first 'subsetSize' indices
    indices.resize(subsetSize);
    return indices;
}

void FlowShop::clampValue(ddouble& val) {
    if(val < -1.0)
        val = 1.0
    else if(val > 1.0)
        val = 1.0
}

std::vector<double> mutate(
    const std::vector<std::vector<double>>& population,
    const std::vector<size_t>& subset,
    int targetIndex,
    double f,
) {
    std::vector<double> mutated(population[targetIndex].size());

    // Create mutated vector
    for(int i = 0; i < mutated.size(); i++) {
        mutated[i] = population[subset[1]][i] - population[subset[2]][i];
        mutated[i] *= f;
        mutated[i] += population[subset[0]][i];
        mutated[i].clampValue();
    }

    return mutated;
}

void FlowShop::crossover(
    std::vector<double>& target,
    const std::vector<double>& mutant,
    double cr,
    MersenneTwister& mt
) {
    int jrand = builder.randNum(0, target.size());

    for(int i = 0; i < target.size(); i++) {
        if (i == jrand || x + mt.genrand_real() % (y - x) < CR)
            target[i] = mutant[i];
    }
}


FlowShopResult FlowShop::runDE(
    size_t popSize,
    double f,
    double cr,
    size_t maxGenerations,
    bool blocking,
    bool optimizeTardiness,
    unsigned long seed
) {
    // Generate random initial population with 1 NEH permutation
    std::vector<std::vector<double>> population(popSize);
    initPopulation(
        population,
        blocking,
        optimizeTardiness,
        seed
    )

    // Stores permutation population representations
    std::vector<std::vector<double>> permutations(popSize);

    // Stores rank of each population member (makespan or tardiness)
    std::vector<uint64_t> solutionRanks(,
        popSize,
        std::numeric_limits<uint64_t>::max();
    );

    // Track global best solution
    size_t globalBestIdx;
    uint64_t globalBestRank = std::numeric_limits<uint64_t>::max();


    // Calculate permutations and rank for initial population
    // Parallelize with OpenMP
    #pragma omp parallel {
        // Best solution found per thread
        uint64_t threadBestRank = -std::numeric_limits<uint64_t>::infinity();
        size_t threadBestIdx = 0;

        // Calculate permutation and rank in parallel
        #pragma omp for
        for(size_t i = 0; i < popSize; i++) {
            // Calculate permutation and rank
            permutations[i] = spvToPerm(population[i]);
            solutionRanks[i] = evaluateSchedule(
                permutations[i],
                blocking,
                optimizeTardiness
            )

            // Compare solution to thread best
            if(solutionRanks[i] < thread_max) {
                // Update thread best
                threadBestRank = solutionRanks[i];
                threadBestIdx = i;
            }
        }

        // Perform reduction to calculate global best solution
        #pragma omp critical {
            // Compare thread best to global best
            if(threadBestRank < globalBestRank) {
                // Update global best
                globalBestRank = threadBestRank;
                globalBestIdx = threadBestIdx;
            }
        }
    }

    // Store temporary population during generation updates
    std::vector<std::vector<double>> newPopulation;
    std::vector<std::vector<size_t>> newPermutations;
    std::vector<size_t>> newSolutionRanks;


    for(size_t i = 0; i < maxGenerations; i++) {
        // Copy generation's initial population
        newPopulation = population;

        for(size_t j = 0; j < popSize; j++) {
            // Create RNG seeded for each iteration
            MersenneTwister mt;
            mt.init_genrand(seed + i); // Ensure determinissm

            // Select 3 distinct solutions
            std::vector<size_t> solutionIndexes = getSubset(popSize, 3, j, mt);

            // Generate mutant vector
            std::vector<double> mutated = mutate(
                population,
                solutionIndexes,
                i,
                f
            )

            // Generate Trial vector
            crossover(newPopulation[i], mutant, cr, mt);

            // Convert trial vector to permutation

            // Calculate permutation ranking

            // Perform greedy replacement

            // Update global best
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
    for(size_t i = 0; i < num_jobs; i++) {
        std::span<uint64_t> job = getJob(i, jobOrder);
        updateCompletions(job, completionTimes, i, blocking);
    }

    if(optimizeTardiness)
        return calculateTardiness(completionTimes, jobOrder);

    return completionTimes[num_jobs_ - 1, num_machines_ - 1];
}


uint64_t FlowShop::calculateTotalTardiness(
    const std::vector<std::vector<uint64_t>>& completionTimes,
    const std::vector<size_t>& jobOrder
) {
    uint64_t totalTardiness = 0;

    // Iterate through jobs
    for(size_t i = 0; i < num_jobs_; i++) {
        // Get job completion time
        uint64_t completeTime = completionTimes[jobOrder[i]][num_machines_ - 1];

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
        uint64_t completeTime = completionTimes[jobOrder[i]][num_machines_ - 1];

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
    std::vector<size_t>& jobOrder
) {
    return { jobs_times_.data() + jobOrder[jobNum] * num_machines_, num_machines_ };
}
#include "FlowShop/FlowShop.h"

#include <algorithm>
#include <limits>


FlowShopResult FlowShop::runNEH(bool blocking) {
    // Create vector to store total execution time for each job
    std::vector<uint64_t> totalJobTimes(num_jobs_, 0);
    computeRowSums(totalJobTimes);

    // Create vector to store job execution order
    std::vector<size_t> jobOrder(num_jobs_);
    argSortJobs(totalJobTimes, jobOrder);

    // Create vector to store completions times of each job
    std::vector<std::vector<uint64_t>> completionTimes(num_jobs_, std::vector<uint64_t>(num_machines_));

    // Calculate completion times for initial job
    std::span<uint64_t> firstJob = getJob(0, jobOrder);
    updateCompletions(firstJob, completionTimes, 0, blocking);

    // Insert remaining jobs
    for(size_t i = 1; i < num_jobs_; i++)
        insertJob(completionTimes, jobOrder, i, blocking);

    // Construct Result object
    FlowShopResult result{
        jobOrder,
        completionTimes[num_jobs_ - 1][num_machines_ - 1],
        completionTimes
    };
    
    return result;
}


void FlowShop::insertJob(
    std::vector<std::vector<uint64_t>>& completionTimes,
    std::vector<size_t>& jobOrder,
    size_t jobNum,
    bool blocking
) {
    // Global best makespan
    uint64_t globalMinMakespan = std::numeric_limits<uint64_t>::max();
    int64_t bestPos = -1; // Optimal insert position

    // Store completion times after inserting next job optimally
    std::vector<std::vector<uint64_t>> bestCompletion;

    // Get next job to insert
    std::span<uint64_t> nextJob = getJob(jobNum, jobOrder);

    #pragma omp parallel // Parallelize block with OpenMP
    {
        // Create thread-local variables to track best insert found in thread
        uint64_t localMinMakespan = std::numeric_limits<uint64_t>::max();
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

            // Derive candidate makespan from completion times
            uint64_t candidateMakespan =
                candidateTimes[jobNum][num_machines_ - 1];

            // Check if this insert position is local thread-best
            if(candidateMakespan < localMinMakespan) {
                // Update thread local best
                localMinMakespan = candidateMakespan;
                localBestPos = i;
                localBestCompletion = std::move(candidateTimes);
            }
        }

        // Perform reduction to determine global best
        #pragma omp critical
        {
            // Compare thread best makespan to global best
            if(localMinMakespan < globalMinMakespan) {
                // Update global best
                globalMinMakespan = localMinMakespan;
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
    std::vector<size_t>& jobOrder
) {
    // Initialize index array
    #pragma omp parallel for
    for(size_t i = 0; i < jobOrder.size(); ++i)
        jobOrder[i] = i;

    std::sort(
        jobOrder.begin(),
        jobOrder.end(),
        [&totalJobTimes](size_t a, size_t b) {
            return totalJobTimes[a] > totalJobTimes[b];
        }
    );
}


std::span<uint64_t> FlowShop::getJob(
    size_t jobNum,
    std::vector<size_t>& jobOrder
) {
    return { jobs_times_.data() + jobOrder[jobNum] * num_machines_, num_machines_ };
}
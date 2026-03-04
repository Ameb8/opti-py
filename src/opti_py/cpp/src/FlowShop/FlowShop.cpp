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
    updateCompletions(firstJob, completionTimes, 0);

    // Insert remaining jobs
    for(size_t i = 1; i < num_jobs_; i++)
        insertJob(completionTimes, jobOrder, i);

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
    size_t jobNum
) {
    uint64_t minMakespan = std::numeric_limits<uint64_t>::max();

    // Iterate through insert positions
    for(int64_t i = jobNum; i >= 0; i--) {
        // create copy of completion times
        std::vector<std::vector<uint64_t>> candidateTimes(completionTimes);

        // Get next job
        std::span<uint64_t> nextJob = getJob(jobNum, jobOrder);

        // Insert next job
        updateCompletions(nextJob, candidateTimes, i);

        // Recalculate completion times after insertion
        for(size_t j = i + 1; j <= jobNum; j++) {
            // Get job to re-insert
            std::span<uint64_t> job = getJob(j - 1, jobOrder);

            updateCompletions(job, candidateTimes, j);
        }

        // Check if makespan improved
        uint64_t candidateMakespan = candidateTimes[jobNum][num_machines_ - 1];

        // Update best insert position
        if(candidateMakespan < minMakespan) {
            completionTimes = candidateTimes;

            // Update job order
            size_t jobId = jobOrder[jobNum];
            for(size_t j = jobNum; j > i; j--)
                jobOrder[j] = jobOrder[j-1];

            jobOrder[i] = jobId; // Set insert position
        }
    }


}

void FlowShop::updateCompletions(
    const std::span<uint64_t> jobTimes,
    std::vector<std::vector<uint64_t>>& completionTimes,
    size_t insertRow
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
    }
}


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
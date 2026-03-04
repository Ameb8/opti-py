#include "FlowShop/FlowShop.h"

#include <algorithm>


FlowShopResult FlowShop::runNEH(bool blocking) {
    FlowShopResult result;

    // Create vector to store total execution time for each job
    std::vector<uint64_t> totalJobTimes(num_jobs_, 0);
    computeRowSums(totalJobTimes);

    // Create vector to store job execution order
    std::vector<size_t> jobOrder(num_jobs_);
    argSortJobs(totalJobTimes, jobOrder);

    // Create vector to store completions times of each job
    std::vector<std::vector<uint64_t>> completionTimes(num_jobs_, std::vector<uint64_t>(num_machines_));

    // Get first job
    std::span<uint64_t> firstJob = getJob(0, jobOrder);
    
    // Calculate completion time for first job
    completionTimes[0][0] = firstJob[0];
    for(size_t i = 1; i < num_machines_; i++)
        completionTimes[0][i] = firstJob[i] + completionTimes[0][i-1];

    

    for(size_t i = 1; i < num_jobs_; i++) {
        insertJob(completionTimes, i);
    }

    // Construct Result object

    return result;
}


void FlowShop::insertJob(
    std::vector<std::vector<uint64_t>> completionTimes,
    size_t jobNum
) {
    // Iterate through insert positions
    for(size_t i = jobNum; i >= 0; i--) {
        // Create matrix for candidate completion times
        std::vector<std::vector<uint64_t>> candidateTimes(
            completionTimes.begin() + i,
            completionTimes.begin() + jobNum + 1
        );

        // Calculate completion times for new row
        candidateTimes[0][0] = completionTimes[i][0]
        for(size_t j = 0; j < num_machines_; j++) {

        }

        // Recalculate execution times for insert pos and after
        for(size_t j = 1; j < candidateTimes.size(); j++) {
            
        }
    }


}


uint64_t executeJobs(
    std::vector<uint64_t>& job1,
    std::vector<uint64_t>& job2,
    std::vector<uint64_t>& compTimes1,
    std::vector<uint64_t>& compTimes2
) {
    // Compute job 1 completion times
    compTimes1[0] = job1[0];
    for(size_t i = 1; i < job1.size(); i++)
        compTimes1[i] = compTimes1[i-1] + job1[i];

    // Compute job 2 completion times
    compTimes2[0] = compTimes1[0] + job2[0];
    for(size_t i = 0; i < job2.size(); i++)
        compTimes2[i] = std::max(compTimes2[i-1], compTimes2[i]) + job2[i];

    // Return makespan (completion time of final job)
    return compTimes2[compTimes2.size() - 1];
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
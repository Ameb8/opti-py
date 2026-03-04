#ifndef FLOW_SHOP_H
#define FLOW_SHOP_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <span>

#include "FlowShop/FlowShopResult.h"

class FlowShop {
public:
    // Constructors
    FlowShop()
        : num_jobs_(0),
          num_machines_(0) {}

    FlowShop(size_t num_jobs,
             size_t num_machines)
        : num_jobs_(num_jobs),
          num_machines_(num_machines),
          jobs_times_(num_jobs * num_machines, 0) {}

    // Internal constructor used by pybind11
    FlowShop(size_t num_jobs,
             size_t num_machines,
             std::vector<uint64_t>&& data)
        : num_jobs_(num_jobs),
          num_machines_(num_machines),
          jobs_times_(std::move(data))
    {
        if (jobs_times_.size() != num_jobs_ * num_machines_)
            throw std::runtime_error("Data size mismatch");
    }

    // Accessors 
    size_t num_jobs() const { return num_jobs_; }
    size_t num_machines() const { return num_machines_; }

    uint64_t* data() { return jobs_times_.data(); }
    const uint64_t* data() const { return jobs_times_.data(); }

    // Replace entire matrix (used by setter)
    void set_jobs(size_t num_jobs,
                  size_t num_machines,
                  std::vector<uint64_t>&& new_data)
    {
        if (new_data.size() != num_jobs * num_machines)
            throw std::runtime_error("Data size mismatch");

        num_jobs_ = num_jobs;
        num_machines_ = num_machines;
        jobs_times_ = std::move(new_data);
    }

    FlowShopResult runNEH(bool blocking = false);

private:
    size_t num_jobs_; // rows
    size_t num_machines_; // columns
    std::vector<uint64_t> jobs_times_; // row-major flattened

    // Helper methods
    void computeRowSums(std::vector<uint64_t>& totalJobTimes);
    void argSortJobs(
        std::vector<uint64_t>& totalJobTimes,
        std::vector<size_t>& jobOrder
    );
    std::span<uint64_t> getJob(
        size_t jobNum,
        std::vector<size_t>& jobOrder
    );
    void insertJob(
        std::vector<std::vector<uint64_t>> completionTimes,
        size_t jobNum
    );
};

#endif
/**
 * @file FlowShop.h
 * @author Alex Buckley
 * @ingroup FlowShop
 * @brief Implementation of the flow shop scheduling problem.
 */

#ifndef FLOW_SHOP_H
#define FLOW_SHOP_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <span>

#include "FlowShop/FlowShopResult.h"


/**
 * @class FlowShop
 * @brief Implements a flow shop problem with support for NEH heuristic optimization.
 *
 * The FlowShop class supports initializing a flow shop problem with a problem config.
 * It allows users to modify job, job times, and machine configuration. The problem 
 * can be optimized by the NEH heuristic, either with or without blocking. Optimization
 * produces a FlowShopResult object containing results
 */
class FlowShop {
public:
    // Constructors
    FlowShop()
        : num_jobs_(0),
          num_machines_(0),
          due_dates_(0) {}

    FlowShop(size_t num_jobs,
             size_t num_machines)
        : num_jobs_(num_jobs),
          num_machines_(num_machines),
          jobs_times_(num_jobs * num_machines, 0),
          due_dates_(num_jobs, 0) {}

    // Internal constructor used by pybind11
    FlowShop(size_t num_jobs,
             size_t num_machines,
             std::vector<uint64_t>&& data)
        : num_jobs_(num_jobs),
          num_machines_(num_machines),
          jobs_times_(std::move(data)),
          due_dates_(num_jobs, 0)
    {
        if (jobs_times_.size() != num_jobs_ * num_machines_)
            throw std::runtime_error("Data size mismatch");
    }

    // Accessors 
    size_t num_jobs() const { return num_jobs_; }
    size_t num_machines() const { return num_machines_; }

    uint64_t* data() { return jobs_times_.data(); }
    const uint64_t* data() const { return jobs_times_.data(); }

    uint64_t* due_dates_data() { return due_dates_.data(); }
    const uint64_t* due_dates_data() const { return due_dates_.data(); }

    // Replace entire matrix (used by setter)
    void set_jobs(
        size_t num_jobs,
        size_t num_machines,
        std::vector<uint64_t>&& new_data
    ) {
        // Throw error if size mismatch
        if(new_data.size() != num_jobs * num_machines)
            throw std::runtime_error("Data size mismatch");

        num_jobs_ = num_jobs;
        num_machines_ = num_machines;
        jobs_times_ = std::move(new_data);
        due_dates_.assign(num_jobs_, 0);
    }

    // Replace due_dates vector
    void set_due_dates(std::vector<uint64_t>&& new_data) {
        // Throw error if size mismatch
        if(new_data.size() != num_jobs_)
            throw std::runtime_error("due_dates size must equal num_jobs");

        due_dates_ = std::move(new_data);
    }

    // Optimize flow shop with NEH heuristic
    FlowShopResult runNEH(
        bool blocking = false, 
        bool optimizeTardiness = false
    );

    // Optimize flow shop with Differential Evolution
    FlowShopResult runDE(
        bool blocking,
        bool optimizeTardiness,
        size_t popSize,
        double f,
        double cr,
        size_t maxGenerations,
        unsigned long seed,
        std::string& mutationStrategy,
        std::string& crossoverStrategy 
    );

    // EvaluateProblem interface (used by DE)
    void initPopulationVectors(
        std::vector<std::vector<double>>& population,
        bool blocking,
        bool optimizeTardiness,
        unsigned long seed
    );
    void getInitialSolutions(std::vector<std::vector<double>>& population);
    constexpr double getLowerBounds() const { return -1.0; }
    constexpr double getUpperBounds() const { return 1.0; }
    double evaluateSolution(const std::vector<double>& solution);
private:
    // Problem inputs
    size_t num_jobs_; // rows
    size_t num_machines_; // columns
    std::vector<uint64_t> jobs_times_; // row-major flattened
    std::vector<uint64_t> due_dates_;

    // State fields set during DE
    bool blocking;
    bool optimizeTardiness;
    unsigned long seed;

    // Produce results
    FlowShopResult buildResult(
        std::vector<size_t> jobOrder,
        bool blocking,
        bool optimizeTardiness,
        std::vector<double> ranksß
    );

    // NEH helper methods
    void computeRowSums(std::vector<uint64_t>& totalJobTimes);
    void argSortJobs(
        std::vector<uint64_t>& totalJobTimes,
        std::vector<size_t>& jobOrder,
        bool sortDueDates
    );
    std::span<uint64_t> getJob(
        size_t jobNum,
        const std::vector<size_t>& jobOrder
    );
    void updateCompletions(
        const std::span<uint64_t> jobTimes,
        std::vector<std::vector<uint64_t>>& completionTimes,
        size_t insertRow,
        bool blocking
    );
    void insertJob(
        std::vector<std::vector<uint64_t>>& completionTimes,
        std::vector<size_t>& jobOrder,
        size_t jobNum,
        bool blocking,
        bool optimizeTardiness
    );

    // Evaluation methods
    std::vector<uint64_t> calculateTardiness(
        const std::vector<std::vector<uint64_t>>& completionTimes,
        const std::vector<size_t>& jobOrder
    );
    uint64_t calculateTotalTardiness(
        const std::vector<std::vector<uint64_t>>& completionTimes,
        const std::vector<size_t>& jobOrder
    );
    uint64_t evaluateSchedule(
        const std::vector<size_t>& jobOrder,
        bool blocking,
        bool optimizeTardiness
    );
    
    // SPV conversion helpers
    std::vector<double> permToSPV(const std::vector<size_t>& permutation);
    void spvToPerm(
        const std::vector<double>& spvVec,
        std::vector<size_t>& permutation
    );

};

#endif
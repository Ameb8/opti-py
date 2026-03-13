/**
 * @file FlowShop.h
 * @author Alex Buckley
 * @brief Flow shop scheduling problem implementation.
 * @defgroup FlowShop Flow Shop Scheduling
 * @ingroup Problems
 *
 * Provides a complete implementation of the classical flow shop scheduling problem
 * with support for both traditional NEH heuristic optimization and metaheuristic
 * optimization via Differential Evolution. Supports makespan minimization and
 * tardiness minimization objectives, with optional job blocking constraints.
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
 * @ingroup FlowShop
 * @brief Implements the classical flow shop scheduling problem.
 *
 * The FlowShop class models a manufacturing environment where jobs must be processed
 * sequentially through a series of machines in a fixed order. The goal is to find
 * an optimal job scheduling order that minimizes either makespan (total completion time)
 * or total tardiness (sum of lateness over all jobs).
 *
 * **Problem Definition:**
 *
 * Given:
 * - n jobs, each with execution times on m machines
 * - Optional due dates for each job
 * - Two objective functions: makespan or total tardiness
 * - Optional blocking constraint (job cannot leave a machine until next is free)
 *
 * Find: A permutation of jobs that minimizes the chosen objective.
 *
 * **Key Features:**
 *
 * - Dual optimization support: NEH heuristic (fast approximation) and DE (high-quality)
 * - Two objective metrics: makespan minimization and tardiness minimization
 * - Optional job blocking for realistic manufacturing constraints
 * - Intelligent population initialization (NEH seed + perturbations + random)
 * - SPV (Smallest Position Value) encoding for seamless DE integration
 * - Thread-safe evaluation with OpenMP parallelization
 *
 * **Problem Data:**
 *
 * - **jobs_times_:** m×n matrix (row-major flattened) storing processing times.
 *   jobs_times_[i * num_machines + j] = time for job i on machine j.
 * - **due_dates_:** Vector of n due dates, one per job (for tardiness calculation).
 *
 * **Evaluable Concept:**
 *
 * FlowShop satisfies the Evaluable concept, allowing it to be optimized directly
 * by DifferentialEvolution:
 * - getInitialSolutions(): Generates population with NEH seeding
 * - evaluateSolution(): Converts SPV vector to job order and evaluates schedule
 * - getLowerBounds()/getUpperBounds(): Returns [-1.0, 1.0] for SPV encoding
 *
 * @note The problem assumes uniform bounds across dimensions (SPV encoding).
 * @note All evaluation is deterministic once scheduling parameters are set (blocking, tardiness).
 */

class FlowShop {
public:
    /** @name Constructors */
    ///@{
 

    /**
     * @brief Constructs a FlowShop problem with zero dimensions.
     *
     * Creates an empty problem instance. Data must be populated via set_jobs() before optimization.
     */
    FlowShop()
        : num_jobs_(0),
          num_machines_(0),
          due_dates_(0) {}
    
    
    /**
     * @brief Constructs a FlowShop problem with specified dimensions.
     *
     * Allocates storage for a problem with @p num_jobs jobs and @p num_machines machines.
     * All processing times initialize to zero; call set_jobs() to populate actual data.
     *
     * @param num_jobs Number of jobs in the scheduling problem
     * @param num_machines Number of machines in sequence
     * @throw std::runtime_error if num_jobs or num_machines is zero
     */
    FlowShop(size_t num_jobs,
             size_t num_machines)
        : num_jobs_(num_jobs),
          num_machines_(num_machines),
          jobs_times_(num_jobs * num_machines, 0),
          due_dates_(num_jobs, 0) {}


    /**
     * @brief Internal constructor used by pybind11 bindings.
     *
     * Constructs a FlowShop with pre-allocated data moved from caller. Validates
     * that data size matches the dimensions (num_jobs × num_machines).
     *
     * @param num_jobs Number of jobs
     * @param num_machines Number of machines
     * @param data Rvalue reference to flattened m×n matrix (row-major)
     * @throw std::runtime_error if data.size() != num_jobs * num_machines
     *
     * @note This constructor is primarily for Python interoperability; use other
     *       constructors for C++ code unless specifically integrating with pybind11.
     */
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

    
    ///@}
 
    /** @name Data Access */
    ///@{
 

    /**
     * @brief Returns the number of jobs in this problem.
     * @return Number of jobs (rows in processing time matrix)
     */
    size_t num_jobs() const { return num_jobs_; }


    /**
     * @brief Returns the number of machines in this problem.
     * @return Number of machines (columns in processing time matrix)
     */
    size_t num_machines() const { return num_machines_; }
 

    /**
     * @brief Returns mutable pointer to flattened processing times matrix.
     *
     * The matrix is stored in row-major order: jobs_times[i * num_machines + j]
     * contains the processing time of job i on machine j.
     *
     * @return Pointer to first element of jobs_times_ vector
     */
    uint64_t* data() { return jobs_times_.data(); }


    /**
     * @brief Returns const pointer to flattened processing times matrix.
     * @return Const pointer to first element of jobs_times_ vector
     * @see data()
     */
    const uint64_t* data() const { return jobs_times_.data(); }
 

    /**
     * @brief Returns mutable pointer to due dates vector.
     *
     * The vector has size num_jobs; due_dates_[i] is the due date for job i.
     * Used for tardiness calculation: tardiness = max(0, completion_time - due_date).
     *
     * @return Pointer to first element of due_dates_ vector
     */
    uint64_t* due_dates_data() { return due_dates_.data(); }
    
    
    /**
     * @brief Returns const pointer to due dates vector.
     * @return Const pointer to first element of due_dates_ vector
     * @see due_dates_data()
     */
    const uint64_t* due_dates_data() const { return due_dates_.data(); }


    ///@}
 
    /** @name Problem Configuration */
    ///@{

 
    /**
     * @brief Replaces the entire processing times matrix and dimensions.
     *
     * Sets new problem dimensions and processing time data. Validates that the
     * data size matches dimensions. Resets due_dates to zero for all jobs.
     *
     * @param num_jobs New number of jobs
     * @param num_machines New number of machines
     * @param new_data Rvalue reference to flattened m×n matrix (row-major, size = num_jobs × num_machines)
     * @throw std::runtime_error if new_data.size() != num_jobs * num_machines
     *
     * @post num_jobs_ and num_machines_ are updated
     * @post jobs_times_ contains the new data
     * @post due_dates_ is reset to [0, 0, ..., 0]
     */
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


    /**
     * @brief Replaces the due dates vector.
     *
     * Sets new due dates for tardiness-based optimization. Must have exactly num_jobs elements.
     *
     * @param new_data Rvalue reference to due dates vector (size = num_jobs)
     * @throw std::runtime_error if new_data.size() != num_jobs_
     *
     * @post due_dates_ contains the new due date values
     *
     * @note Due dates are only used when optimizeTardiness=true in runNEH() or runDE().
     */
    void set_due_dates(std::vector<uint64_t>&& new_data) {
        // Throw error if size mismatch
        if(new_data.size() != num_jobs_)
            throw std::runtime_error("due_dates size must equal num_jobs");

        due_dates_ = std::move(new_data);
    }

    
    ///@}
 
    /** @name Optimization Methods */
    ///@{
 
    /**
     * @brief Optimizes the flow shop schedule using the NEH heuristic.
     *
     * The Nawaz-Enscore-Ham (NEH) algorithm is a polynomial-time approximation
     * algorithm that produces high-quality solutions quickly:
     *
     * 1. **Initialization:** Sort jobs by total processing time (or due date).
     * 2. **Insertion:** Iteratively insert each remaining job at the position
     *    that minimizes the chosen objective (makespan or tardiness).
     * 3. **Output:** Return the final job order and computed metrics.
     *
     * **Complexity:** O(n² m) for makespan; O(n² m) for tardiness with due dates.
     *
     * @param blocking If true, enforces job blocking: a job cannot leave machine i
     *                 until machine i+1 is free (models buffer constraints)
     * @param optimizeTardiness If true, minimizes total tardiness (sum of max(0, completion - due_date));
     *                          if false, minimizes makespan (final completion time)
     *
     * @return FlowShopResult containing:
     *         - sequence: Job order (indices 0 to num_jobs-1)
     *         - makespan: Total completion time of final job on final machine
     *         - completionTimes: m×n matrix of completion times
     *         - tardiness: Vector of tardiness for each job (if optimizeTardiness=true)
     *         - fitnesses: Empty vector (NEH does not track convergence)
     *
     * **Example:**
     * ```cpp
     * FlowShop problem(10, 5);  // 10 jobs, 5 machines
     * // ... populate jobs_times and due_dates ...
     * FlowShopResult result = problem.runNEH(false, true);  // Tardiness, no blocking
     * ```
     */
    FlowShopResult runNEH(
        bool blocking = false, 
        bool optimizeTardiness = false
    );


    /**
     * @brief Optimizes the flow shop schedule using Differential Evolution.
     *
     * Applies the DE metaheuristic to find high-quality solutions. The algorithm:
     * 1. Initializes population with NEH seed, perturbations, and random solutions
     * 2. Iterates over generations, applying mutation and crossover
     * 3. Evaluates each candidate by converting SPV encoding to job order
     * 4. Returns best solution found and convergence history
     *
     * **Solution Encoding:** SPV (Smallest Position Value)
     * - Continuous vector in [-1, 1]^n representing job priorities
     * - Higher values = earlier scheduling position
     * - Enables DE's continuous operators on permutation problem
     *
     * **Population Initialization:**
     * - 1 solution: NEH-generated schedule (converted to SPV)
     * - 25% of population: NEH with small random perturbations
     * - Remaining: Uniformly random solutions in [-1, 1]^n
     *
     * @param blocking If true, enforces job blocking constraints
     * @param optimizeTardiness If true, minimizes tardiness; if false, minimizes makespan
     * @param popSize Population size; typically 20-50 for small problems
     * @param f Differential weight (mutation scale factor); typical range (0, 2], e.g., 0.8
     * @param cr Crossover rate; range [0, 1], e.g., 0.9 (higher = more diversity)
     * @param maxGenerations Maximum iterations; termination criterion
     * @param seed Random seed for reproducible results
     * @param mutationStrategy Name of mutation operator; one of:
     *                         "rand1", "rand2", "best1", "best2", "randToBest1"
     *                         Defaults to "rand1" if empty
     * @param crossoverStrategy Name of crossover operator; one of:
     *                          "bin" (binomial, default), "exp" (exponential)
     *                          Defaults to "bin" if empty
     *
     * @return FlowShopResult containing:
     *         - sequence: Best job order found
     *         - makespan: Makespan of best solution
     *         - completionTimes: Completion times for best solution
     *         - tardiness: Tardiness vector for best solution
     *         - fitnesses: Convergence history (best fitness per generation)
     *
     * **Thread Safety:** Evaluation is parallelized with OpenMP. Concurrent solution
     * evaluations are safe; problem state (blocking, optimizeTardiness) is read-only
     * during optimization.
     *
     * **Example:**
     * ```cpp
     * FlowShop problem(20, 10);
     * // ... set jobs and due dates ...
     * FlowShopResult result = problem.runDE(
     *     true,              // blocking
     *     true,              // optimize tardiness
     *     30,                // population size
     *     0.8,               // F
     *     0.9,               // CR
     *     100,               // generations
     *     42,                // seed
     *     mutationStrategy = "best1",
     *     crossoverStrategy = "bin"
     * );
     * ```
     */
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


    ///@}
 
    /** @name Evaluable Concept Implementation */
    ///@{


    /**
     * @brief Generates an initial population of candidate solutions.
     *
     * Creates @p population.size() SPV-encoded solutions using three strategies:
     * 1. First solution: NEH heuristic schedule
     * 2. Next 25%: NEH with small random perturbations
     * 3. Remaining: Uniformly random in [-1, 1]^n
     *
     * This initialization biases the population toward high-quality regions (NEH solutions)
     * while maintaining diversity for exploration.
     *
     * @param[out] population Pre-allocated vector of population.size() solution vectors.
     *                        Each solution is filled with SPV values in [-1, 1].
     *
     * **Thread Safety:** Uses OpenMP parallelization to generate initial solutions quickly.
     *
     * @note This method requires blocking and optimizeTardiness fields to be set
     *       (typically by runDE() before calling DifferentialEvolution::optimize).
     *
     * @see initPopulationVectors()
     */
    void getInitialSolutions(std::vector<std::vector<double>>& population);


    /**
     * @brief Returns the lower bound for the optimization domain.
     *
     * For SPV encoding, all dimensions share a uniform lower bound of -1.0.
     *
     * @return Constant value -1.0
     */
    constexpr double getLowerBounds() const { return -1.0; }
    
    
    /**
     * @brief Returns the upper bound for the optimization domain.
     *
     * For SPV encoding, all dimensions share a uniform upper bound of 1.0.
     *
     * @return Constant value 1.0
     */

    constexpr double getUpperBounds() const { return 1.0; }


    /**
     * @brief Evaluates the objective function for a given solution.
     *
     * Converts an SPV-encoded vector to a job permutation, evaluates the resulting
     * schedule, and returns the objective value (makespan or tardiness).
     *
     * **Algorithm:**
     * 1. Convert SPV vector to job order via spvToPerm()
     * 2. Calculate completion times for the schedule
     * 3. Return makespan (if optimizeTardiness=false) or total tardiness (if true)
     *
     * **Complexity:** O(nm) for completion time calculation + O(n log n) for SPV conversion.
     *
     * @param[in] solution SPV-encoded solution vector (size = num_jobs, values in [-1, 1])
     *
     * @return Objective value:
     *         - Makespan (completion time of last job) if optimizeTardiness=false
     *         - Total tardiness (sum of individual tardiness) if optimizeTardiness=true
     *         Returned as double for compatibility with DE optimizer.
     *
     * **Thread Safety:** This method is thread-safe for concurrent evaluation when
     * blocking and optimizeTardiness fields are constant. Must not be called
     * concurrently with runDE() or field modifications.
     *
     * **Requirements:** blocking and optimizeTardiness must be set before calling
     * (typically by runDE() prior to optimizer invocation).
     */

    double evaluateSolution(const std::vector<double>& solution);

    ///@}

private:
    /** @name Problem Data */
    ///@{
 
    /// Number of jobs in the scheduling problem
    size_t num_jobs_;
 
    /// Number of machines in the production sequence
    size_t num_machines_;
 
    /// Processing times matrix (row-major flattened): jobs_times_[i * num_machines + j]
    /// = time for job i on machine j. Size: num_jobs * num_machines.
    std::vector<uint64_t> jobs_times_;
 
    /// Due dates for each job. Size: num_jobs.
    /// Used for tardiness calculation: tardiness[i] = max(0, completion_time[i] - due_date[i]).
    std::vector<uint64_t> due_dates_;
 
    ///@}


    /** @name Optimization State */
    ///@{
 
    /// Whether job blocking is enforced during current optimization
    bool blocking;
 
    /// Whether optimizing tardiness (true) or makespan (false) during current optimization
    bool optimizeTardiness;
 
    /// Random seed for current optimization (used in population initialization)
    unsigned long seed;
 
    ///@}

 
    /** @name NEH Heuristic Helpers */
    ///@{

    
    /**
     * @brief Constructs a FlowShopResult from a job order and convergence history.
     *
     * Evaluates the complete schedule for the given job order, calculating makespan,
     * completion times, and tardiness vector. Packages results into a FlowShopResult object.
     *
     * @param[in] jobOrder Job permutation (indices 0 to num_jobs-1)
     * @param[in] blocking Whether blocking is enforced
     * @param[in] optimizeTardiness Whether tardiness is optimized
     * @param[in] ranks Convergence history vector (best fitness per generation)
     *
     * @return FlowShopResult containing all computed metrics
     *
     * @post Returned result has valid sequence, makespan, completionTimes, tardiness
     */
    FlowShopResult buildResult(
        std::vector<size_t> jobOrder,
        bool blocking,
        bool optimizeTardiness,
        std::vector<double> ranks
    );

 
    /**
     * @brief Computes total processing time for each job across all machines.
     *
     * Parallelized computation: totalJobTimes[i] = sum of jobs_times_[i * num_machines + j]
     * for all j. Used by NEH to initialize job ordering.
     *
     * @param[out] totalJobTimes Output vector (size = num_jobs) to store sums
     */
    void computeRowSums(std::vector<uint64_t>& totalJobTimes);

    /**
     * @brief Sorts jobs by total processing time or due date.
     *
     * Initializes jobOrder permutation and sorts by:
     * - Due date (descending) if sortDueDates=true
     * - Total processing time (descending) if sortDueDates=false
     *
     * @param[in] totalJobTimes Precomputed total times for each job (used if !sortDueDates)
     * @param[out] jobOrder Permutation vector to fill with sorted job indices
     * @param[in] sortDueDates If true, sort by due date; else by total processing time
     */
    void argSortJobs(
        std::vector<uint64_t>& totalJobTimes,
        std::vector<size_t>& jobOrder,
        bool sortDueDates
    );


    /**
     * @brief Returns a span over a single job's processing times.
     *
     * Convenience accessor for job i in the ordered sequence:
     * Returns jobs_times_[jobOrder[i] * num_machines .. jobOrder[i] * num_machines + num_machines)
     *
     * @param[in] jobNum Position in the current job order (0 to jobNum-1)
     * @param[in] jobOrder Current job permutation
     *
     * @return std::span of length num_machines containing processing times for the job
     */
    std::span<uint64_t> getJob(
        size_t jobNum,
        const std::vector<size_t>& jobOrder
    );


    /**
     * @brief Updates completion times when a job is inserted into the schedule.
     *
     * Computes completion times for a job at position insertRow:
     * - For row 0: completion[0][j] = job[0] + completion[0][j-1]
     * - For row > 0: completion[insertRow][j] = job[j] + max(completion[insertRow-1][j],
     *                                                          completion[insertRow][j-1])
     * - If blocking: propagates backward to enforce buffer constraints
     *
     * @param[in] jobTimes Processing times for this job (span of length num_machines)
     * @param[in,out] completionTimes Matrix to update (size: num_jobs × num_machines)
     * @param[in] insertRow Row index to update
     * @param[in] blocking If true, enforce job blocking after computing standard completion times
     */
    void updateCompletions(
        const std::span<uint64_t> jobTimes,
        std::vector<std::vector<uint64_t>>& completionTimes,
        size_t insertRow,
        bool blocking
    );


    /**
     * @brief Inserts a job at the optimal position in the partial schedule.
     *
     * NEH's core step: for each remaining job, test all insertion positions
     * and choose the one that best optimizes the objective (makespan or tardiness).
     * Parallelized with OpenMP for efficiency.
     *
     * **Algorithm:**
     * 1. For each possible insertion position i in [0, jobNum]
     * 2. Compute completion times with job inserted at position i
     * 3. Evaluate objective (makespan or total tardiness)
     * 4. Thread-safe reduction to find global best position
     * 5. Update jobOrder and completionTimes with best insertion
     *
     * @param[in,out] completionTimes Completion times to update in place
     * @param[in,out] jobOrder Job permutation to update
     * @param[in] jobNum Index of job to insert (1 to num_jobs-1)
     * @param[in] blocking Whether to enforce blocking
     * @param[in] optimizeTardiness Whether to optimize tardiness (else makespan)
     *
     * @post jobOrder is updated with jobNum inserted at optimal position
     * @post completionTimes is updated to reflect the new insertion
     */
    void insertJob(
        std::vector<std::vector<uint64_t>>& completionTimes,
        std::vector<size_t>& jobOrder,
        size_t jobNum,
        bool blocking,
        bool optimizeTardiness
    );


    ///@}
 
    /** @name Objective Evaluation */
    ///@{
 

    /**
     * @brief Evaluates a job order to compute its objective value.
     *
     * Simulates the schedule for the given job permutation and returns either
     * makespan or total tardiness based on optimizeTardiness flag.
     *
     * @param[in] jobOrder Job sequence (permutation of 0 to num_jobs-1)
     * @param[in] blocking Whether to apply job blocking
     * @param[in] optimizeTardiness If true, return total tardiness; else makespan
     *
     * @return Objective value: makespan (timestamp) or total tardiness (time units)
     *
     * @see evaluateSchedule() performs the core computation
     */
    uint64_t evaluateSchedule(
        const std::vector<size_t>& jobOrder,
        bool blocking,
        bool optimizeTardiness
    );


    /**
     * @brief Calculates individual tardiness for each job.
     *
     * Tardiness[i] = max(0, completion_time[i] - due_date[jobOrder[i]])
     * Measures how late each job is relative to its due date.
     *
     * @param[in] completionTimes Completion times matrix (num_jobs × num_machines)
     * @param[in] jobOrder Job permutation used in scheduling
     *
     * @return Vector of tardiness values (size: num_jobs)
     */
    std::vector<uint64_t> calculateTardiness(
        const std::vector<std::vector<uint64_t>>& completionTimes,
        const std::vector<size_t>& jobOrder
    );
 
    /**
     * @brief Calculates total tardiness across all jobs.
     *
     * TotalTardiness = sum of max(0, completion_time[i] - due_date[jobOrder[i]]) for all i.
     * Single aggregated objective for tardiness minimization.
     *
     * @param[in] completionTimes Completion times matrix
     * @param[in] jobOrder Job permutation
     *
     * @return Sum of tardiness across all jobs
     */
    uint64_t calculateTotalTardiness(
        const std::vector<std::vector<uint64_t>>& completionTimes,
        const std::vector<size_t>& jobOrder
    );
 
    ///@}



    
    ///@}
 
    /** @name SPV Encoding Conversion */
    ///@{
 
    /**
     * @brief Converts a job permutation to its SPV (Smallest Position Value) encoding.
     *
     * SPV encoding maps permutations to continuous vectors in [-1, 1]^n, enabling
     * DE's continuous operators on the permutation problem.
     *
     * **Algorithm:**
     * 1. For each job i, assign spvVec[jobOrder[i]] = position of i in the order
     * 2. Scale positions from [0, n-1] to [-1, 1]
     *
     * **Decoding Rule:** To recover permutation, sort jobs by SPV value (descending).
     *
     * **Example:**
     * - Permutation: [2, 0, 1] (job 2 first, then 0, then 1)
     * - Positions: job 0 at pos 1, job 1 at pos 2, job 2 at pos 0
     * - SPV (before scaling): [1, 2, 0]
     * - SPV (after scaling to [-1, 1]): [-1, 1, -1]
     *
     * @param[in] permutation Job ordering (size = num_jobs)
     *
     * @return SPV vector (size = num_jobs, values in [-1, 1])
     *
     * @see spvToPerm() for the inverse operation
     */
    std::vector<double> permToSPV(const std::vector<size_t>& permutation);
 
    /**
     * @brief Converts an SPV vector to its job permutation via sorting.
     *
     * Recovers the job order from SPV encoding by sorting job indices by their
     * SPV values in descending order (higher values = earlier positions).
     * Ties are broken by job index (smaller index wins) for determinism.
     *
     * **Algorithm:**
     * 1. Initialize permutation as [0, 1, ..., n-1]
     * 2. Sort by spvVec values in descending order
     * 3. Secondary sort by job index for tie-breaking
     *
     * @param[in] spvVec SPV-encoded solution (size = num_jobs, values in [-1, 1])
     * @param[out] permutation Job ordering to fill (size = num_jobs)
     *
     * @post permutation is sorted by descending SPV values (with tie-breaking)
     *
     * @see permToSPV() for the inverse operation
     */
    void spvToPerm(
        const std::vector<double>& spvVec,
        std::vector<size_t>& permutation
    );
 
    ///@}

       /** @name Population Initialization */
    ///@{
 
    /**
     * @brief Generates an initial population of candidate solutions for DE.
     *
     * Creates @p population.size() SPV-encoded job orders using a hybrid strategy:
     * 1. **First solution:** NEH heuristic result (converted to SPV)
     * 2. **Next 25%:** NEH solution with small random perturbations
     *    (perturbation scale: 0.1 for makespan, 0.02 for tardiness)
     * 3. **Remaining:** Uniformly random SPV vectors in [-1, 1]^n
     *
     * This initialization balances exploitation (starting from NEH) with exploration
     * (random diversity), accelerating convergence while maintaining search breadth.
     *
     * **Thread Safety:** Uses OpenMP to parallelize random solution generation.
     *
     * @param[out] population Pre-allocated vector of solution vectors (size set by caller)
     * @param[in] blocking Blocking constraint flag (passed to runNEH())
     * @param[in] optimizeTardiness Tardiness flag (affects perturbation scale and NEH behavior)
     * @param[in] seed Random seed for deterministic initialization
     *
     * @post Each population[i] is filled with num_jobs SPV values in [-1, 1]
     *
     * @note This method is called by getInitialSolutions(), which is in turn called
     *       by DifferentialEvolution::optimize().
     */
    void initPopulationVectors(
        std::vector<std::vector<double>>& population,
        bool blocking,
        bool optimizeTardiness,
        unsigned long seed
    );
 
    ///@}
};

#endif
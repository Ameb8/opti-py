/**
 * @file FlowShopResult.h
 * @author Alex Buckley
 * @brief Result object for flow shop scheduling optimization.
 * @defgroup FlowShopResult Flow Shop Results
 * @ingroup Problems
 *
 * Defines the FlowShopResult structure that encapsulates all outputs from
 * flow shop scheduling optimizations (NEH heuristic or Differential Evolution).
 * Provides complete visibility into the solution quality and schedule details.
 */

#ifndef FLOW_SHOP_RESULT_H
#define FLOW_SHOP_RESULT_H

#include <vector>
#include <cstddef>
#include <cstdint>


/**
 * @struct FlowShopResult
 * @ingroup FlowShopResult
 * @brief Encapsulates the complete output of a flow shop scheduling optimization.
 *
 * FlowShopResult contains all relevant data from a flow shop optimization run,
 * including the best job sequence found, timing metrics, and (for metaheuristic
 * optimizations) the convergence history showing how fitness improved over time.
 *
 * **Output from NEH Heuristic:**
 *
 * When calling FlowShop::runNEH(), the result contains:
 * - **sequence:** The job ordering produced by the NEH algorithm
 * - **makespan:** Total completion time (final machine finish time)
 * - **completionTimes:** Detailed completion times for each job on each machine
 * - **tardiness:** Per-job tardiness values (if optimizeTardiness=true)
 * - **fitnesses:** Empty vector (NEH is deterministic, no convergence history)
 *
 * **Output from Differential Evolution:**
 *
 * When calling FlowShop::runDE(), the result contains:
 * - **sequence:** The best job ordering found by DE
 * - **makespan:** Makespan of the best solution
 * - **completionTimes:** Completion times for the best solution
 * - **tardiness:** Per-job tardiness for the best solution (if optimizeTardiness=true)
 * - **fitnesses:** Convergence history (best fitness per generation), enables
 *   tracking optimization progress and assessing convergence quality
 *
 * **Data Relationships:**
 *
 * The fields are internally consistent and derived from a single optimal
 * job sequence. Specifically:
 * - **completionTimes** is computed from **sequence** according to flow shop rules
 * - **makespan** = completionTimes[num_jobs-1][num_machines-1] (final completion time)
 * - **tardiness[i]** = max(0, completionTimes[i][num_machines-1] - due_date[sequence[i]])
 * - **fitnesses** tracks the best objective value (makespan or tardiness) per generation
 *
 * **Usage Example:**
 *
 * ```cpp
 * FlowShop problem(20, 10);
 * // ... populate problem data ...
 *
 * // Optimize with DE and analyze results
 * FlowShopResult result = problem.runDE(true, true, 30, 0.8, 0.9, 100, 42, mut, cross);
 *
 * // Print final solution
 * std::cout << "Best makespan: " << result.makespan << std::endl;
 * std::cout << "Job sequence: ";
 * for (size_t job : result.sequence)
 *     std::cout << job << " ";
 * std::cout << std::endl;
 *
 * // Analyze convergence
 * std::cout << "Initial fitness: " << result.fitnesses.front() << std::endl;
 * std::cout << "Final fitness: " << result.fitnesses.back() << std::endl;
 * std::cout << "Improvement: " << (result.fitnesses.front() - result.fitnesses.back()) << std::endl;
 * ```
 *
 * **Thread Safety:**
 *
 * FlowShopResult is not thread-safe for concurrent modifications. However,
 * once fully constructed by the optimizer, it can be safely read by multiple
 * threads without synchronization.
 *
 * @note All vectors are guaranteed to have consistent sizes after construction:
 *       - sequence.size() = num_jobs
 *       - completionTimes.size() = num_jobs, each row has num_machines elements
 *       - tardiness.size() = num_jobs
 *       - fitnesses.size() = num_generations (0 for NEH, maxGenerations for DE)
 *
 * @see FlowShop::runNEH() for NEH output semantics
 * @see FlowShop::runDE() for DE output semantics
 */
struct FlowShopResult {
    /** @name Solution */
    ///@{

    /**
     * @brief Job execution sequence (job ordering).
     *
     * A permutation of job indices [0, num_jobs-1] representing the optimal
     * schedule found by the optimizer. The vector has length num_jobs.
     *
     * **Interpretation:**
     * - sequence[0] = the first job to schedule
     * - sequence[1] = the second job to schedule
     * - sequence[i] = the job at position i in the schedule
     *
     * **Example:**
     * If sequence = [2, 0, 1], the execution order is: job 2, then job 0, then job 1.
     *
     * **Size:** num_jobs (the number of jobs in the problem)
     *
     * **Valid Range:** Each element is in [0, num_jobs-1], all elements are distinct.
     *
     * @note This sequence is derived from the optimizer's SPV encoding by sorting
     *       jobs by their SPV values in descending order.
     */
    std::vector<size_t> sequence;

    ///@}

    /** @name Timing Metrics */
    ///@{

    /**
     * @brief Total completion time (makespan) of the schedule.
     *
     * The time at which the final job finishes on the final machine.
     * Represents the total elapsed time from the start of the first job until
     * the completion of the last job.
     *
     * **Definition:**
     * makespan = completionTimes[num_jobs - 1][num_machines - 1]
     *
     * **Unit:** Time units as specified in the problem data (seconds, minutes, etc.)
     *
     * **Semantics:**
     * - Lower values are better (minimization objective)
     * - Achievable lower bound: max(total processing time / num_machines,
     *   longest job, longest path through all machines)
     * - For makespan minimization, this is the primary objective optimized
     *
     * **Example:**
     * If a problem with 3 jobs and 2 machines has:
     * - Job 0: [5, 10] time units
     * - Job 1: [3, 7]
     * - Job 2: [8, 4]
     * With sequence [1, 0, 2], makespan might be 29 (the final job finishes at t=29).
     *
     * @invariant makespan >= 0
     * @invariant makespan = completionTimes.back().back() (if completionTimes is filled)
     */
    uint64_t makespan;

    /**
     * @brief Completion times matrix: when each job finishes on each machine.
     *
     * A matrix where completionTimes[i][j] represents the time at which the
     * job at position i in the sequence completes on machine j.
     *
     * **Dimensions:** num_jobs rows × num_machines columns
     *
     * **Indexing:**
     * - completionTimes[i] = completion times for the job at position i in sequence
     * - completionTimes[i][j] = finish time of position-i job on machine j
     *
     * **Example:**
     * For a 3-job, 2-machine problem:
     * ```
     * completionTimes = [
     *     [5, 15],   // Job at position 0 finishes at t=5 on M1, t=15 on M2
     *     [8, 22],   // Job at position 1 finishes at t=8 on M1, t=22 on M2
     *     [16, 26]   // Job at position 2 finishes at t=16 on M1, t=26 on M2
     * ]
     * ```
     *
     * **Flow Shop Properties:**
     * - completionTimes[i][0] >= completionTimes[i-1][0] (machines process in sequence)
     * - completionTimes[i][j] >= completionTimes[i][j-1] (job finishes on next machine)
     * - completionTimes[i][j] >= completionTimes[i-1][j] (machine respects previous job)
     *
     * **Blocking Constraint:**
     * If blocking is enabled, additional constraint:
     * - completionTimes[i][j] >= completionTimes[i-1][j+1] (buffer constraint)
     *
     * **Tardiness Calculation:**
     * tardiness[i] = max(0, completionTimes[i][num_machines-1] - due_date[sequence[i]])
     *
     * **Size:** num_jobs × num_machines
     *
     * @note All values are non-negative and non-decreasing along rows and columns
     *       (except for blocking constraints which may create more complex patterns).
     * @note The final element completionTimes.back().back() equals makespan.
     */
    std::vector<std::vector<uint64_t>> completionTimes;

    ///@}

    /** @name Tardiness Metrics */
    ///@{

    /**
     * @brief Per-job tardiness values (lateness from due dates).
     *
     * A vector where tardiness[i] represents how late the job at position i
     * is relative to its due date.
     *
     * **Definition:**
     * tardiness[i] = max(0, completionTimes[i][num_machines-1] - due_date[sequence[i]])
     *
     * **Semantics:**
     * - 0 = job completed on time (no lateness)
     * - > 0 = job is late by this many time units
     * - Sum of all tardiness values is the total tardiness objective
     *
     * **Unit:** Time units (same as completion times)
     *
     * **Example:**
     * If due_dates = [20, 30, 25] and jobs finish at [15, 35, 22]:
     * ```
     * tardiness = [
     *     max(0, 15 - 20) = 0,    // Job 0 on time
     *     max(0, 35 - 30) = 5,    // Job 1 late by 5
     *     max(0, 22 - 25) = 0     // Job 2 on time
     * ]
     * ```
     *
     * **Size:** num_jobs (length matches sequence and completionTimes rows)
     *
     * **When Populated:**
     * - Always filled by runNEH() and runDE()
     * - Non-zero values only if due dates are meaningful (not all zero)
     *
     * **Related Objective:**
     * - Sum of tardiness = std::accumulate(tardiness.begin(), tardiness.end(), 0)
     * - Used for tardiness minimization objective
     *
     * @invariant tardiness[i] >= 0 for all i
     * @invariant tardiness.size() = sequence.size()
     * @note This is computed from completionTimes and due dates;
     *       modifying it directly will not affect makespan or completionTimes.
     */
    std::vector<uint64_t> tardiness;

    ///@}

    /** @name Optimization Progress */
    ///@{

    /**
     * @brief Convergence history: best fitness per generation.
     *
     * A vector tracking the best objective value found at each generation
     * during metaheuristic optimization. Enables analysis of optimization progress
     * and assessment of convergence quality.
     *
     * **Semantics:**
     * - fitnesses[i] = best objective value found through generation i (inclusive)
     * - fitnesses[0] = best value in generation 0 (initial population)
     * - fitnesses[num_generations-1] = final best value (equals makespan or total tardiness)
     * - Vector is monotone non-increasing (or non-decreasing for maximization)
     *
     * **Content:**
     * - For makespan minimization: fitness = makespan (time units)
     * - For tardiness minimization: fitness = total tardiness (time units)
     *
     * **Size:**
     * - **NEH heuristic:** empty vector (size 0); NEH is deterministic
     * - **Differential Evolution:** size = maxGenerations; one value per generation
     *
     * **Example:**
     * For a DE run with maxGenerations=100:
     * ```
     * fitnesses.size() = 100
     * fitnesses[0] = 1500       // Initial best (from population)
     * fitnesses[50] = 1200      // Best after 50 generations
     * fitnesses[99] = 1050      // Final best value
     *
     * // Improvement over 100 generations:
     * improvement = fitnesses[0] - fitnesses[99] = 450 time units
     * ```
     *
     * **Analysis Use Cases:**
     *
     * 1. **Convergence Speed:**
     *    ```cpp
     *    size_t generations_to_converge = 0;
     *    for (size_t i = 0; i < result.fitnesses.size(); ++i) {
     *        if (result.fitnesses[i] == result.fitnesses.back()) {
     *            generations_to_converge = i;
     *            break;
     *        }
     *    }
     *    ```
     *
     * 2. **Convergence Quality:**
     *    ```cpp
     *    double initial = result.fitnesses.front();
     *    double final = result.fitnesses.back();
     *    double improvement = initial - final;
     *    double improvement_percent = 100.0 * improvement / initial;
     *    ```
     *
     * 3. **Stagnation Detection:**
     *    ```cpp
     *    bool converged_early = result.fitnesses[50] == result.fitnesses.back();
     *    ```
     *
     * **When to Use:**
     * - To assess optimization quality and performance
     * - To tune algorithm parameters (F, CR, popSize, etc.)
     * - To compare different mutation/crossover strategies
     * - To detect if more generations are needed
     *
     * **When Empty:**
     * - If produced by FlowShop::runNEH() (heuristics don't have convergence history)
     * - Check size before accessing: `if (!result.fitnesses.empty())`
     *
     * @invariant fitnesses is empty for NEH results
     * @invariant fitnesses.size() = maxGenerations for DE results
     * @invariant fitnesses[i] >= fitnesses[i+1] (monotone decreasing for minimization)
     * @invariant fitnesses.back() = makespan or total_tardiness (final best value)
     *
     * @note To get total tardiness: std::accumulate(result.tardiness.begin(),
     *       result.tardiness.end(), 0ull)
     */
    std::vector<double> fitnesses;

    ///@}
};


#endif
/**
 * @file OptResult.h
 * @author Alex Buckley
 * @brief Result object for Differential Evolution optimization.
 * @defgroup OptResult DE Optimization Results
 * @ingroup DifferentialEvolution
 *
 * Defines the OptResult structure that encapsulates all outputs from
 * Differential Evolution optimization runs, including the best solution found,
 * final fitness value, and convergence history.
 */


#ifndef OPT_RESULT_H
#define OPT_RESULT_H

#include <vector>


/**
 * @struct OptResult
 * @ingroup OptResult
 * @brief Encapsulates the complete output of a Differential Evolution optimization run.
 *
 * OptResult contains all relevant data produced by DifferentialEvolution::optimize(),
 * including the best solution discovered, its fitness value, and the convergence
 * trajectory showing how fitness improved over generations.
 *
 * **Output from DifferentialEvolution::optimize():**
 *
 * When calling DifferentialEvolution::optimize(), the returned OptResult contains:
 * - **bestSolution:** The parameter vector with lowest fitness found
 * - **bestFitness:** The fitness value of bestSolution (scalar)
 * - **bestFitnesses:** Convergence history (best fitness per generation)
 *
 * **Data Relationships:**
 *
 * The fields are internally consistent:
 * - **bestFitness** = minimum value in **bestFitnesses** (final value)
 * - **bestFitnesses[i]** = best fitness found through generation i (inclusive)
 * - **bestFitnesses** is non-increasing (monotone decreasing for minimization)
 * - **bestSolution.size()** = problem dimension
 * - **bestFitnesses.size()** = maxGenerations (from optimizer)
 *
 * **Usage Example:**
 *
 * ```cpp
 * // Create problem and run DE
 * MyOptimizationProblem problem;
 * OptResult result = DifferentialEvolution::optimize(
 *     problem,
 *     popSize = 30,
 *     F = 0.8,
 *     CR = 0.9,
 *     maxGenerations = 100,
 *     seed = 42,
 *     mutationStrategy = "best1",
 *     crossoverStrategy = "bin"
 * );
 *
 * // Access solution
 * std::cout << "Best fitness: " << result.bestFitness << std::endl;
 * std::cout << "Solution: ";
 * for (double x : result.bestSolution)
 *     std::cout << x << " ";
 * std::cout << std::endl;
 *
 * // Analyze convergence
 * std::cout << "Initial fitness: " << result.bestFitnesses.front() << std::endl;
 * std::cout << "Final fitness: " << result.bestFitnesses.back() << std::endl;
 * std::cout << "Improvement: " 
 *     << (result.bestFitnesses.front() - result.bestFitnesses.back()) 
 *     << std::endl;
 * ```
 *
 * **Thread Safety:**
 *
 * OptResult is not thread-safe for concurrent modifications. However, once fully
 * constructed by the optimizer, it can be safely read by multiple threads without
 * synchronization.
 *
 * **Typical Size:**
 *
 * For a 30-dimensional problem with 100 generations:
 * - bestSolution.size() = 30
 * - bestFitnesses.size() = 100
 * - Memory usage: ~30 doubles + 100 doubles ≈ 1 KB
 *
 * @note All vectors are guaranteed to be non-empty after successful optimization.
 * @note bestFitness is always the last (best) value in bestFitnesses.
 * @note All fitness values are monotone non-increasing (for minimization).
 *
 * @see DifferentialEvolution::optimize() for method that produces this result
 */
struct OptResult {
    /** @name Solution */
    ///@{

    /**
     * @brief The best parameter vector found during optimization.
     *
     * The parameter vector that achieved the lowest (best) fitness value
     * during the entire optimization run. This is the primary output of
     * the optimizer representing the solution to the optimization problem.
     *
     * **Content:**
     * - Each element is a parameter/variable to be optimized
     * - Values are constrained to domain [lowerBound, upperBound]
     * - Dimension matches the optimization problem dimension
     *
     * **Size:** problem.dimension
     * - For 30-dimensional problem: size = 30
     * - For 100-dimensional problem: size = 100
     *
     * **Semantics:**
     * - These are the x-values that minimize f(x)
     * - Each element is a continuous value in the domain
     * - Order and meaning depend on problem definition
     *
     * **Example:**
     *
     * For a 3D Rosenbrock minimization:
     * ```cpp
     * // Rosenbrock: f(x,y,z) = (1-x)² + 100(y-x²)² + (1-z)² + 100(z-y²)²
     * // Optimum at (1, 1, 1) with fitness 0
     * result.bestSolution = [0.9999, 0.9998, 1.0001]  // Close to optimum
     * result.bestFitness = 0.000042  // Near-optimal fitness
     * ```
     *
     * **Access Pattern:**
     *
     * ```cpp
     * // Iterate through solution
     * for (size_t i = 0; i < result.bestSolution.size(); ++i) {
     *     double param = result.bestSolution[i];
     *     // Use parameter in your application
     * }
     *
     * // Or with range-based for (C++11+)
     * for (double param : result.bestSolution) {
     *     // Use parameter
     * }
     * ```
     *
     * **Post-Processing:**
     *
     * You may want to post-process the solution:
     * ```cpp
     * // Clamp to exact bounds if numerical precision drifted
     * for (auto& x : result.bestSolution)
     *     x = std::clamp(x, lowerBound, upperBound);
     *
     * // Round if integer solution expected
     * for (auto& x : result.bestSolution)
     *     x = std::round(x);
     * ```
     *
     * @invariant bestSolution.size() = problem dimension
     * @invariant All elements are within [lowerBound, upperBound]
     * @invariant fitness(bestSolution) = bestFitness
     * @note This is non-empty after successful optimization
     */
    std::vector<double> bestSolution;

    ///@}

    /** @name Fitness Values */
    ///@{

    /**
     * @brief Convergence history: best fitness per generation.
     *
     * A record of the best fitness value found at each generation during
     * the optimization. Enables analysis of how the optimization progressed
     * and assessment of convergence behavior.
     *
     * **Content:**
     * - bestFitnesses[i] = best fitness found through generation i (inclusive)
     * - Lower values are better (minimization problem)
     * - Values are monotone non-increasing (or plateau)
     *
     * **Size:** maxGenerations (from optimizer call)
     * - For 100 generation run: size = 100
     * - For 1000 generation run: size = 1000
     *
     * **Indexing:**
     * - bestFitnesses[0] = best fitness in generation 0 (initial population)
     * - bestFitnesses[50] = best fitness after 50 generations
     * - bestFitnesses[99] = best fitness in generation 99 (final)
     *
     * **Invariant Properties:**
     * - Monotone non-increasing: bestFitnesses[i] >= bestFitnesses[i+1]
     * - Starts with initial population fitness
     * - Ends with final best value: bestFitnesses.back() == bestFitness
     *
     * **Example Convergence:**
     *
     * ```
     * Generation 0:   fitness = 1500.0  (random initial population)
     * Generation 10:  fitness = 1200.5  (improvement)
     * Generation 50:  fitness = 850.3   (good improvement)
     * Generation 99:  fitness = 425.1   (final best)
     *
     * bestFitnesses = [1500.0, 1499.2, 1498.1, ..., 850.3, ..., 425.1]
     * Size = 100, all non-increasing
     * ```
     *
     * **Analysis Use Cases:**
     *
     * 1. **Convergence Speed:**
     *    ```cpp
     *    size_t generations_to_converge = 0;
     *    for (size_t i = 0; i < result.bestFitnesses.size(); ++i) {
     *        if (result.bestFitnesses[i] == result.bestFitness) {
     *            generations_to_converge = i;
     *            break;
     *        }
     *    }
     *    ```
     *
     * 2. **Convergence Quality:**
     *    ```cpp
     *    double initial = result.bestFitnesses.front();
     *    double final = result.bestFitnesses.back();
     *    double improvement = initial - final;
     *    double improvement_percent = 100.0 * improvement / initial;
     *    ```
     *
     * 3. **Stagnation Detection:**
     *    ```cpp
     *    // Check if optimization stalled after halfway point
     *    size_t halfway = result.bestFitnesses.size() / 2;
     *    bool stalled = (result.bestFitnesses[halfway] == result.bestFitness);
     *    ```
     *
     * 4. **Learning Curve:**
     *    ```cpp
     *    // Plot fitnesses per generation to visualize convergence curve
     *    for (size_t gen = 0; gen < result.bestFitnesses.size(); ++gen) {
     *        double fitness = result.bestFitnesses[gen];
     *        std::cout << gen << "," << fitness << "\n";
     *    }
     *    ```
     *
     * 5. **Performance Assessment:**
     *    ```cpp
     *    // Compare two runs' convergence
     *    auto convergence_ratio = [](const OptResult& r) {
     *        double init = r.bestFitnesses.front();
     *        double final = r.bestFitnesses.back();
     *        return (init - final) / init;  // Improvement ratio
     *    };
     *    double ratio1 = convergence_ratio(result1);
     *    double ratio2 = convergence_ratio(result2);
     *    ```
     *
     * **Visualization:**
     *
     * ```cpp
     * // Log convergence for plotting
     * std::ofstream file("convergence.csv");
     * for (size_t gen = 0; gen < result.bestFitnesses.size(); ++gen) {
     *     file << gen << "," << result.bestFitnesses[gen] << "\n";
     * }
     * file.close();
     *
     * // Then plot with your favorite tool:
     * // gnuplot: plot "convergence.csv" using 1:2 with lines
     * ```
     *
     * **Parameter Tuning Based on Convergence:**
     *
     * ```cpp
     * // If stagnating early, increase exploration
     * if (result.bestFitnesses[50] == result.bestFitness) {
     *     // Run with larger F or higher CR
     * }
     *
     * // If slow convergence, increase exploitation
     * if (result.bestFitness > acceptable_threshold) {
     *     // Run with smaller F or lower CR, or use Best1 mutation
     * }
     * ```
     *
     * @invariant bestFitnesses.size() = maxGenerations
     * @invariant bestFitnesses[i] >= bestFitnesses[i+1] (non-increasing)
     * @invariant bestFitnesses.back() == bestFitness (last value is best)
     * @invariant All values are finite (not NaN or infinity)
     * @note Used to track optimization progress over time
     * @note Essential for understanding algorithm behavior
     * @note Enables convergence diagnostics and tuning
     */
    std::vector<double> bestFitnesses;

    /**
     * @brief The best (lowest) fitness value found during optimization.
     *
     * The minimum fitness achieved by the optimizer across all generations.
     * This scalar represents the quality of the solution.
     *
     * **Semantics:**
     * - Lower values are better (minimization problem)
     * - This is the fitness of bestSolution
     * - Objective function value: f(bestSolution) = bestFitness
     *
     * **Relationship to bestFitnesses:**
     * - bestFitness = bestFitnesses.back() (final value)
     * - bestFitness = minimum element in bestFitnesses
     * - bestFitness achieved at some generation index
     *
     * **Example:**
     *
     * For Sphere function minimization:
     * ```cpp
     * // Sphere: f(x) = sum(x_i²), optimum at x = 0 with f = 0
     * result.bestFitness = 0.0042  // Final best fitness
     * result.bestSolution = [0.04, 0.02, 0.03, ...]  // Parameters
     * // Verify: sum([0.04²,0.02²,0.03²,...]) ≈ 0.0042 ✓
     * ```
     *
     * **Convergence Assessment:**
     *
     * ```cpp
     * // Check if optimization reached target
     * double target_fitness = 0.001;
     * if (result.bestFitness <= target_fitness) {
     *     std::cout << "Target reached!" << std::endl;
     * } else {
     *     std::cout << "Target not reached. Gap: " 
     *         << (result.bestFitness - target_fitness) << std::endl;
     * }
     * ```
     *
     * **Comparison Between Runs:**
     *
     * ```cpp
     * // Run optimization multiple times, track best results
     * std::vector<OptResult> results;
     * for (int seed = 0; seed < 10; ++seed) {
     *     results.push_back(DifferentialEvolution::optimize(..., seed));
     * }
     *
     * // Find best run overall
     * auto best_run = std::min_element(
     *     results.begin(), results.end(),
     *     [](const OptResult& a, const OptResult& b) {
     *         return a.bestFitness < b.bestFitness;
     *     }
     * );
     * ```
     *
     * **Statistical Analysis:**
     *
     * ```cpp
     * // For multiple runs
     * std::vector<OptResult> results;  // Filled from multiple optimizations
     * std::vector<double> fitnesses;
     *
     * for (const auto& result : results) {
     *     fitnesses.push_back(result.bestFitness);
     * }
     *
     * // Compute statistics
     * double mean = std::accumulate(fitnesses.begin(), fitnesses.end(), 0.0) 
     *             / fitnesses.size();
     * double min = *std::min_element(fitnesses.begin(), fitnesses.end());
     * double max = *std::max_element(fitnesses.begin(), fitnesses.end());
     *
     * std::cout << "Best: " << min << ", Mean: " << mean << ", Worst: " << max;
     * ```
     *
     * **Unit and Scale:**
     *
     * - Units depend on objective function (user-defined)
     * - Scale depends on problem (can be 0.001 to 1e9)
     * - Always comparable within same problem
     * - May need normalization for cross-problem comparisons
     *
     * @invariant bestFitness = min(bestFitnesses)
     * @invariant bestFitness is finite (not NaN or infinity)
     * @invariant bestFitness is the objective function value at bestSolution
     * @note This is the primary metric of solution quality
     * @note Lower values are better
     */
    double bestFitness;

    ///@}
};


#endif
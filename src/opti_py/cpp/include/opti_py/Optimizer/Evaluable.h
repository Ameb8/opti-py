/**
 * @file Evaluable.h
 * @author Alex Buckley
 * @brief Evaluable concept for optimization problem definitions.
 * @defgroup Concepts Concept Definitions
 * @ingroup Optimizers
 *
 * Defines the Evaluable concept that all optimization problems must satisfy
 * to be compatible with the DifferentialEvolution optimizer and other
 * numerical optimization algorithms.
 */
 
#pragma once
#include <vector>

/**
 * @concept Evaluable
 * @ingroup Concepts
 * @brief Concept defining the interface for optimization problems.
 *
 * The Evaluable concept specifies the required methods and semantics that
 * an optimization problem must provide to be compatible with metaheuristic
 * optimizers like Differential Evolution.
 *
 * **Semantic Requirements:**
 *
 * An Evaluable type models a continuous optimization problem over a rectangular
 * domain [lower, upper]^n. The optimizer's task is to find the vector
 * **x** = (x₁, x₂, ..., xₙ) that minimizes the scalar objective function
 * f(**x**), where each xᵢ ∈ [lower, upper].
 *
 * **Method Requirements:**
 *
 * A type T satisfies Evaluable if it provides the following methods:
 *
 * 1. **getInitialSolutions()**
 *    ```cpp
 *    void getInitialSolutions(std::vector<std::vector<double>>& population);
 *    ```
 *    - **Purpose:** Generate an initial population of candidate solutions
 *    - **Parameters:**
 *      - @p population: Non-const reference to a pre-allocated vector of popSize solution vectors.
 *        Each solution is a vector of doubles with dimension matching the problem's parameter space.
 *    - **Postcondition:** All solutions are filled with initial candidate values.
 *      Values should be feasible (within [lower, upper]) but can employ any initialization
 *      strategy (random, Latin hypercube, user-provided, etc.).
 *    - **Example:** For a 3D problem with popSize=10, population has size 10,
 *      and each population[i] has size 3.
 *
 * 2. **getLowerBounds()**
 *    ```cpp
 *    double getLowerBounds() const;
 *    ```
 *    - **Purpose:** Return the lower bound of the feasible domain
 *    - **Returns:** A scalar value L such that all problem dimensions have lower bound L.
 *    - **Semantics:** The optimizer constrains all parameters to [L, upper].
 *    - **Note:** Assumes a uniform bound across all dimensions (if per-dimension bounds
 *      are needed, modify this interface to return a vector).
 *    - **Example:** For a problem with domain [-5, 5]^n, returns -5.0.
 *
 * 3. **getUpperBounds()**
 *    ```cpp
 *    double getUpperBounds() const;
 *    ```
 *    - **Purpose:** Return the upper bound of the feasible domain
 *    - **Returns:** A scalar value U such that all problem dimensions have upper bound U.
 *    - **Semantics:** The optimizer constrains all parameters to [lower, U].
 *    - **Example:** For a problem with domain [-5, 5]^n, returns 5.0.
 *
 * 4. **evaluateSolution()**
 *    ```cpp
 *    double evaluateSolution(const std::vector<double>& solution) const;
 *    ```
 *    - **Purpose:** Evaluate the objective function at a given solution
 *    - **Parameters:**
 *      - @p solution: Const reference to a solution vector (dimension matches problem).
 *    - **Returns:** A scalar fitness value (double) representing the objective function.
 *    - **Semantics:** Lower values are considered better (minimization problem).
 *      The optimizer seeks to minimize this value.
 *    - **Thread Safety:** Must be thread-safe if the optimizer uses parallelization (e.g., OpenMP).
 *      Concurrent calls from different threads must not cause data races or undefined behavior.
 *    - **Performance:** Typically the bottleneck; should be optimized for speed.
 *    - **Example:** For Rosenbrock function f(x,y) = (1-x)² + 100(y-x²)²,
 *      evaluateSolution({x, y}) returns that value.
 *
 * **Example Implementation:**
 *
 * ```cpp
 * class RosenbrockProblem {
 * public:
 *     void getInitialSolutions(std::vector<std::vector<double>>& population) {
 *         for (auto& sol : population) {
 *             for (auto& x : sol) {
 *                 x = -2.0 + (rand() / (double)RAND_MAX) * 4.0;  // [-2, 2]
 *             }
 *         }
 *     }
 *
 *     double getLowerBounds() const { return -2.0; }
 *     double getUpperBounds() const { return 2.0; }
 *
 *     double evaluateSolution(const std::vector<double>& x) const {
 *         double result = 0.0;
 *         for (size_t i = 0; i < x.size() - 1; ++i) {
 *             double t1 = 1.0 - x[i];
 *             double t2 = x[i+1] - x[i] * x[i];
 *             result += t1 * t1 + 100.0 * t2 * t2;
 *         }
 *         return result;
 *     }
 * };
 * ```
 *
 * **Usage with DifferentialEvolution:**
 *
 * ```cpp
 * RosenbrockProblem problem;
 * OptResult result = DifferentialEvolution::optimize(
 *     problem,
 *     popSize = 30,
 *     f = 0.8,
 *     cr = 0.9,
 *     maxGenerations = 200,
 *     seed = 42,
 *     mutationStrategy = "best1",
 *     crossoverStrategy = "bin"
 * );
 * ```
 *
 * **Notes on Concept Checks:**
 *
 * The concept uses `std::convertible_to<double>` for getLowerBounds(),
 * getUpperBounds(), and evaluateSolution() return types, allowing implicit
 * conversions (e.g., returning float or int). This provides flexibility while
 * ensuring numeric compatibility.
 *
 * The concept requires const-correctness on const methods (getters and evaluateSolution),
 * enabling the optimizer to use const references to problems and enforce read-only semantics.
 *
 * @note This is a C++20 concept using the <concepts> header.
 *       Ensure your compiler supports concepts and is invoked with -std=c++20 or later.
 */
template<typename T> 
concept Evaluable = requires(T t, std::vector<std::vector<double>>& pop, const std::vector<double>& sol) {
    { t.getInitialSolutions(pop) } -> std::same_as<void>;
    { t.getLowerBounds() } -> std::convertible_to<double>;
    { t.getUpperBounds() } -> std::convertible_to<double>;
    { t.evaluateSolution(sol) } -> std::convertible_to<double>;
};
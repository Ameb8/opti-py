/**
 * @file Rand1.h
 * @author Alex Buckley
 * @brief DE/rand/1 mutation strategy implementation.
 * @ingroup Mutation
 *
 * Implements the classical DE/rand/1 mutation operator, which applies a single
 * weighted difference vector to a random base individual.
 */


#ifndef RAND1_H
#define RAND1_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


/**
 * @class Rand1
 * @ingroup Mutation
 * @brief Implements the DE/rand/1 mutation strategy.
 *
 * **Strategy: DE/rand/1 (Random-1)**
 *
 * The most basic and exploratory DE mutation strategy. Uses a random base vector
 * and applies a single weighted difference vector to generate variation.
 *
 * **Formula:**
 *
 * @f[
 * v_i(t) = x_{r0}(t) + F \cdot (x_{r1}(t) - x_{r2}(t))
 * @f]
 *
 * Where:
 * - v_i: Mutant vector for target i
 * - x_r0, x_r1, x_r2: Three randomly selected distinct individuals (r0 ≠ i)
 * - F: Differential weight controlling step size
 *
 * **Algorithm:**
 *
 * 1. Select 3 random distinct individuals from population (excluding target)
 * 2. For each dimension j:
 *    - mutant[j] = r0[j] + F * (r1[j] - r2[j])
 *    - Clamp mutant[j] to [lowerBound, upperBound]
 * 3. Return the mutant vector
 *
 * **Characteristics:**
 *
 * **Exploration:** Very high
 * - Uses random base instead of best, so explores widely
 * - Single difference vector provides moderate variation
 * - Good for escaping local optima
 *
 * **Exploitation:** Very low
 * - Random base means no bias toward good solutions
 * - Slow convergence to final solution
 * - May waste evaluations on poor regions
 *
 * **Robustness:** Excellent
 * - Least likely to converge prematurely
 * - Performs well on diverse problem landscapes
 * - Good for multimodal problems
 *
 * **Convergence Speed:** Slow
 * - Requires many generations to refine solutions
 * - Well-suited for high-dimensional problems
 * - Best with large population sizes
 *
 * **Parameter Recommendations:**
 *
 * - **F (Differential Weight):**
 *   - Smaller F (0.5-0.8): Finer control, slower convergence
 *   - Larger F (0.8-1.5): Stronger exploration, faster initial progress
 *   - Typical: 0.8-1.0
 *
 * - **CR (Crossover Rate):**
 *   - Lower CR (0.3-0.5): More conservative mixing
 *   - Higher CR (0.7-0.9): More aggressive mixing
 *   - Typical: 0.9-1.0
 *
 * - **Population Size:**
 *   - Minimum: 4 × dimension (needs at least 3 random individuals + target)
 *   - Recommended: 10-20 × dimension
 *   - Larger populations improve robustness
 *
 * **Comparison with Other Strategies:**
 *
 * | Strategy | Base | Differences | Exploration | Convergence |
 * |----------|------|-------------|-------------|-------------|
 * | Rand1    | Random | 1         | Very High   | Very Slow   |
 * | Best1    | Best   | 1         | Low         | Very Fast   |
 * | Rand2    | Random | 2         | Extreme     | Slow        |
 * | Best2    | Best   | 2         | Moderate    | Moderate    |
 * | RandBest1| Hybrid | 1         | Moderate    | Moderate    |
 *
 * **Use Cases:**
 *
 * - Highly multimodal optimization landscapes
 * - When global exploration is prioritized
 * - Problems with unknown structure
 * - As a baseline for algorithm comparison
 * - When premature convergence is a risk
 *
 * **Avoid When:**
 *
 * - Problem is unimodal and smooth (use Best1 instead)
 * - Computational budget is very limited (use Best1)
 * - Need rapid convergence (use Best2 or RandBest1)
 *
 * **Implementation Details:**
 *
 * The implementation follows a straightforward pattern:
 * 1. Call getSubset() to randomly select r0, r1, r2
 * 2. For each dimension, compute: r0 + F * (r1 - r2)
 * 3. Clamp to bounds using std::clamp
 * 4. Return result
 *
 * **Complexity:**
 *
 * Time: O(dimension)
 * Space: O(dimension) for the mutant vector
 *
 * **Example Usage:**
 *
 * ```cpp
 * // Create Rand1 strategy
 * auto strategy = std::make_unique<Rand1>();
 *
 * // Generate mutant vector
 * std::vector<double> mutant = strategy->mutate(
 *     population,     // Current population
 *     7,              // Target individual index
 *     0.8,            // F = 0.8
 *     bestSolution,   // (unused in Rand1)
 *     rng,
 *     -1.0, 1.0       // Domain bounds
 * );
 *
 * // Use mutant for crossover and selection
 * ```
 *
 * **Historical Note:**
 *
 * DE/rand/1 was the original DE mutation strategy proposed by Storn and Price (1997).
 * Despite the availability of more refined strategies, it remains a benchmark for
 * algorithm robustness and is commonly used as the default strategy.
 *
 * @see Mutation for interface documentation
 * @see Best1 for a more exploitative variant
 * @see Rand2 for higher variation
 */

class Rand1 : public Mutation {
public:
    /**
     * @brief Generates a mutant vector using DE/rand/1 strategy.
     *
     * Applies the formula: mutant = r0 + F * (r1 - r2)
     * where r0, r1, r2 are three randomly selected distinct individuals.
     *
     * @param[in] population Current population of solutions
     * @param[in] targetIndex Index of the target individual (excluded from selection)
     * @param[in] F Differential weight; typical range 0.5-1.5
     * @param[in] bestVector Best solution (unused by Rand1, but required by interface)
     * @param[in,out] mt Random number generator for selection
     * @param[in] lowerBound Lower domain bound
     * @param[in] upperBound Upper domain bound
     *
     * @return Mutant vector with all values clamped to [lowerBound, upperBound]
     */
    std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        size_t targetIndex,
        double F,
        const std::vector<double>& bestVector,
        MersenneTwister& mt,
        double lowerBound,
        double upperBound
    ) override {
        std::vector<size_t> subset = getSubset(population.size(), 3, targetIndex, mt);

        // Get population subset        
        const std::vector<double>& r1 = population[subset[0]];
        const std::vector<double>& r2 = population[subset[1]];
        const std::vector<double>& r3 = population[subset[2]];

        std::vector<double> mutated(bestVector.size());

        // Create mutated vector
        for(int i = 0; i < mutated.size(); i++) {
            mutated[i] = population[subset[1]][i] - population[subset[2]][i];
            mutated[i] *= F;
            mutated[i] += population[subset[0]][i];
            mutated[i] = std::clamp(mutated[i], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif

/**
 * @file Best1.h
 * @author Alex Buckley
 * @brief DE/best/1 mutation strategy implementation.
 * @ingroup Mutation
 *
 * Implements the DE/best/1 mutation operator, which applies a single weighted
 * difference vector to the best (lowest fitness) individual found so far.
 */


#ifndef BEST1_H
#define BEST1_H


#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"
#include <algorithm>


/**
 * @class Best1
 * @ingroup Mutation
 * @brief Implements the DE/best/1 mutation strategy.
 *
 * **Strategy: DE/best/1 (Best-1)**
 *
 * A highly exploitative mutation strategy that biases mutations toward the
 * best (lowest fitness) solution found so far. This creates a directed search
 * toward known good regions while still maintaining diversity through difference vectors.
 *
 * **Formula:**
 *
 * @f[
 * v_i(t) = x_{best}(t) + F \cdot (x_{r1}(t) - x_{r2}(t))
 * @f]
 *
 * Where:
 * - v_i: Mutant vector for target i
 * - x_best: Best (lowest fitness) individual in current population
 * - x_r1, x_r2: Two randomly selected distinct individuals (r1 ≠ i, r2 ≠ i)
 * - F: Differential weight controlling step size
 *
 * **Algorithm:**
 *
 * 1. Select 2 random distinct individuals from population (excluding target)
 * 2. For each dimension j:
 *    - mutant[j] = best[j] + F * (r1[j] - r2[j])
 *    - Clamp mutant[j] to [lowerBound, upperBound]
 * 3. Return the mutant vector
 *
 * **Characteristics:**
 *
 * **Exploration:** Very low
 * - Uses best solution as base, biasing search toward known good region
 * - Single difference vector provides minimal variation
 * - Risk of getting stuck in local optima
 *
 * **Exploitation:** Very high
 * - Focuses search efforts in promising regions
 * - Excellent for fine-tuning solutions
 * - Rapid convergence to local optima
 *
 * **Robustness:** Poor to moderate
 * - May converge prematurely on multimodal landscapes
 * - Good on unimodal or smooth problems
 * - Vulnerable to deceptive landscapes
 *
 * **Convergence Speed:** Very fast
 * - Reaches good solutions quickly
 * - Excellent for time-limited problems
 * - May sacrifice final solution quality for speed
 *
 * **Parameter Recommendations:**
 *
 * - **F (Differential Weight):**
 *   - Smaller F (0.4-0.7): Smoother convergence, less risk of overshooting
 *   - Larger F (0.7-1.0): Stronger initial progress, may miss targets
 *   - Typical: 0.7-0.9
 *   - Note: Larger F is less critical than in Rand1 since base is already good
 *
 * - **CR (Crossover Rate):**
 *   - Lower CR (0.5-0.8): More conservative, can reduce diversity
 *   - Higher CR (0.8-1.0): Better diversity, recommended
 *   - Typical: 0.9
 *
 * - **Population Size:**
 *   - Minimum: 3 × dimension (needs at least 2 random individuals + best)
 *   - Recommended: 10-15 × dimension
 *   - Smaller populations acceptable (less computational cost)
 *
 * **Comparison with Other Strategies:**
 *
 * | Strategy | Base | Differences | Exploration | Convergence |
 * |----------|------|-------------|-------------|-------------|
 * | Rand1    | Random | 1         | Very High   | Very Slow   |
 * | Best1    | Best   | 1         | Very Low    | Very Fast   |
 * | Rand2    | Random | 2         | Extreme     | Slow        |
 * | Best2    | Best   | 2         | Moderate    | Moderate    |
 * | RandBest1| Hybrid | 1         | Moderate    | Moderate    |
 *
 * **Use Cases:**
 *
 * - Unimodal or smooth optimization landscapes
 * - When computational budget is limited
 * - Fine-tuning near known solutions
 * - Time-critical optimization
 * - Problems with clear gradients toward optima
 * - Combining with adaptive F values for robustness
 *
 * **Avoid When:**
 *
 * - Problem is highly multimodal
 * - Global exploration is critical
 * - Multiple local optima with significantly different qualities
 * - Limited population diversity is a concern
 *
 * **Synergy with Other Components:**
 *
 * Best1 works best when combined with:
 * - **High CR (0.9-1.0):** Maintains diversity from difference vectors
 * - **Moderate F (0.7-0.9):** Balances convergence and stability
 * - **Appropriate population size:** Large enough to maintain diversity
 * - **Adaptive F/CR:** jDE-style adaptation helps prevent premature convergence
 *
 * **Implementation Details:**
 *
 * The implementation is straightforward:
 * 1. Call getSubset() to randomly select r1, r2
 * 2. Use the provided bestVector as the base
 * 3. For each dimension, compute: best + F * (r1 - r2)
 * 4. Clamp to bounds using std::clamp
 * 5. Return result
 *
 * **Complexity:**
 *
 * Time: O(dimension)
 * Space: O(dimension) for the mutant vector
 *
 * **Example Usage:**
 *
 * ```cpp
 * // Create Best1 strategy
 * auto strategy = std::make_unique<Best1>();
 *
 * // Generate mutant vector
 * std::vector<double> mutant = strategy->mutate(
 *     population,     // Current population
 *     5,              // Target individual index
 *     0.8,            // F = 0.8
 *     bestSolution,   // Best individual found
 *     rng,
 *     -1.0, 1.0       // Domain bounds
 * );
 *
 * // Use mutant for crossover and selection
 * ```
 *
 * **Typical Performance Profile:**
 *
 * - **Rosenbrock function:** Excellent (smooth landscape)
 * - **Rastrigin function:** Poor (highly multimodal)
 * - **Sphere function:** Excellent (simple unimodal)
 * - **Schwefel function:** Moderate (deceptive)
 *
 * **Historical Context:**
 *
 * DE/best/1 is one of the earliest variants developed after the original DE/rand/1.
 * It remains widely used and is the default strategy in many DE implementations.
 * More recent strategies like RandBest1 attempt to balance Best1's speed with
 * Rand1's robustness.
 *
 * @see Mutation for interface documentation
 * @see Rand1 for a more exploratory variant
 * @see Best2 for higher variation
 * @see RandBest1 for a hybrid approach
 */
class Best1 : public Mutation {
public:
    /**
     * @brief Generates a mutant vector using DE/best/1 strategy.
     *
     * Applies the formula: mutant = best + F * (r1 - r2)
     * where r1 and r2 are randomly selected distinct individuals.
     *
     * @param[in] population Current population of solutions
     * @param[in] targetIndex Index of the target individual (excluded from selection)
     * @param[in] F Differential weight; typical range 0.5-1.0
     * @param[in] bestVector The best solution found so far (used as base)
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
        std::vector<size_t> subset = getSubset(population.size(), 2, targetIndex, mt);

        const std::vector<double>& xr1 = population[subset[0]];
        const std::vector<double>& xr2 = population[subset[1]];

        std::vector<double> mutated(bestVector.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = bestVector[j] + F * (xr1[j] - xr2[j]);
            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }
        
        return mutated;
    }
};



#endif

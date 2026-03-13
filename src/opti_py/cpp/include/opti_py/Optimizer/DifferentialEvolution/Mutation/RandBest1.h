/**
 * @file RandBest1.h
 * @author Alex Buckley
 * @brief DE/rand-to-best/1 mutation strategy implementation.
 * @ingroup Mutation
 *
 * Implements the DE/rand-to-best/1 mutation operator, which interpolates between
 * random exploration and best-directed search using an interpolation parameter.
 */


#ifndef RANDBEST1_H
#define RANDBEST1_H


#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


/**
 * @class RandBest1
 * @ingroup Mutation
 * @brief Implements the DE/rand-to-best/1 mutation strategy.
 *
 * **Strategy: DE/rand-to-best/1 (Rand-to-Best-1)**
 *
 * A hybrid mutation strategy that continuously interpolates between random
 * exploration (Rand1) and best-directed search (Best1). This provides a
 * principled way to balance exploration and exploitation, offering robustness
 * without sacrificing convergence speed.
 *
 * **Formula:**
 *
 * @f[
 * v_i(t) = x_i(t) + \lambda \cdot (x_{best}(t) - x_i(t)) + F \cdot (x_{r1}(t) - x_{r2}(t))
 * @f]
 *
 * Where:
 * - v_i: Mutant vector for target i
 * - x_i: Target (current) individual
 * - x_best: Best (lowest fitness) individual in current population
 * - x_r1, x_r2: Two randomly selected distinct individuals
 * - λ: Interpolation parameter (0 = Rand1, 1 = Best1, 0.5 = balanced)
 * - F: Differential weight controlling step size
 *
 * **Decomposition:**
 *
 * The formula can be viewed as three components:
 * 1. **Exploitation:** λ × (best - current) - pulls toward best
 * 2. **Exploration:** F × (r1 - r2) - provides random variation
 * 3. **Anchoring:** current - keeps solution in neighborhood
 *
 * **Algorithm:**
 *
 * 1. Select 2 random distinct individuals from population (excluding target)
 * 2. Compute attraction to best: λ × (best - current)
 * 3. Compute random difference: F × (r1 - r2)
 * 4. For each dimension j:
 *    - mutant[j] = current[j] + λ*(best[j] - current[j]) + F*(r1[j] - r2[j])
 *    - Clamp mutant[j] to [lowerBound, upperBound]
 * 5. Return the mutant vector
 *
 * **Characteristics:**
 *
 * **Exploration (λ-dependent):**
 * - λ = 0.0: Pure Rand1, extreme exploration
 * - λ = 0.5: Balanced exploration-exploitation
 * - λ = 1.0: Pure Best1, minimal exploration
 * - Typical: 0.5-0.8 for good compromise
 *
 * **Exploitation (λ-dependent):**
 * - λ = 0.0: No bias toward best
 * - λ = 0.5: Moderate bias toward best
 * - λ = 1.0: Strong bias toward best
 *
 * **Robustness:**
 * - λ = 0.5: Very good (reduces premature convergence)
 * - λ < 0.5: Excellent (more like Rand1)
 * - λ > 0.5: Good (more like Best1)
 *
 * **Convergence Speed:**
 * - λ = 0.5: Moderate (better than Rand1, slower than Best1)
 * - λ < 0.5: Slow (approaches Rand1)
 * - λ > 0.5: Fast (approaches Best1)
 *
 * **Population Size Requirements:**
 *
 * - **Minimum:** 3 × dimension
 *   - Needs at least 2 random individuals + target
 *   - Does not specifically need best, though best helps convergence
 *
 * - **Recommended:** 10-20 × dimension
 *   - Depends on λ value
 *   - Larger λ (closer to Best1) tolerates smaller populations
 *   - Smaller λ (closer to Rand1) benefits from larger populations
 *
 * **Parameter Recommendations:**
 *
 * - **λ (Interpolation Parameter):**
 *   - **0.3-0.5:** Robust, good for unknown problems
 *   - **0.5-0.7:** Balanced, good for most problems
 *   - **0.7-0.9:** Fast, good for smooth problems
 *   - **Adaptive λ:** Can increase λ over time (as in jDE) for adaptive control
 *   - **Default (our implementation):** 0.8 (slightly exploitative)
 *   - **Note:** Experiment with λ based on problem characteristics
 *
 * - **F (Differential Weight):**
 *   - Smaller F (0.5-0.8): Balanced with λ bias
 *   - Larger F (0.8-1.2): Stronger random component
 *   - Typical: 0.7-1.0
 *   - Interaction: Larger λ allows smaller F; smaller λ benefits from larger F
 *
 * - **CR (Crossover Rate):**
 *   - Lower CR (0.5-0.8): More selective mixing
 *   - Higher CR (0.8-1.0): More aggressive mixing
 *   - Typical: 0.9
 *   - Varies less with λ than with strategy choice
 *
 * **Comparison with Other Strategies:**
 *
 * | Strategy | Base | Differences | λ | Exploration | Convergence | Balance |
 * |----------|------|-------------|---|-------------|-------------|---------|
 * | Rand1    | Rand | 1           | 0 | Very High   | Very Slow   | Robust  |
 * | Best1    | Best | 1           | 1 | Very Low    | Very Fast   | Local   |
 * | Rand2    | Rand | 2           | 0 | Extreme     | Slow        | Robust  |
 * | Best2    | Best | 2           | 1 | Moderate    | Moderate    | Good    |
 * | RandBest1| Hybrid | 1         | λ | Variable    | Variable    | Tunable |
 *
 * **Use Cases:**
 *
 * - **General-purpose optimization** where problem structure is unknown
 * - **Portfolio approaches** as a safe default when one strategy unavailable
 * - **Adaptive schemes** where λ evolves during optimization
 * - **Bridging exploration-exploitation** gap between Rand1 and Best1
 * - **Benchmark comparisons** to test relative strategy performance
 * - **When Best1 converges prematurely** but Rand1 is too slow
 * - **Problems with moderate multimodality** where balanced search helps
 * - **Educational purposes** to understand strategy continuum
 *
 * **Avoid When:**
 *
 * - Extreme robustness required (use Rand2 with λ=0)
 * - Maximum convergence speed needed (use Best1 with λ=1)
 * - λ=0.8 doesn't match problem characteristics (use tuned Best1/Rand1)
 *
 * **Adaptive Usage (jDE-style):**
 *
 * RandBest1 works excellently with parameter adaptation:
 * - **Increasing λ over time:** Start with 0.4, increase toward 0.9 as best improves
 * - **Decreasing F over time:** Start with 1.0, decrease as convergence nears
 * - **Crossover adaptation:** Increase CR if population diversity declines
 *
 * This creates a self-tuning approach that balances exploration early and
 * exploitation late in the optimization run.
 *
 * **Mathematical Interpretation:**
 *
 * The interpolation parameter λ can be understood as:
 * - **Component Weighting:** How much to blend best-attraction vs random-difference
 * - **Search Bias:** Controls whether search is biased toward current, best, or unbiased
 * - **Convergence Rate:** Higher λ leads to faster convergence curves
 * - **Escape Probability:** Lower λ maintains better escape from local optima
 *
 * **Implementation Details:**
 *
 * The implementation combines three search components:
 * 1. Call getSubset() to select 2 random individuals
 * 2. Compute attraction toward best: λ × (best - current)
 * 3. Compute random difference: F × (r1 - r2)
 * 4. Sum all components with current individual
 * 5. Clamp to bounds
 * 6. Return result
 *
 * **Complexity:**
 *
 * Time: O(dimension)
 * Space: O(dimension) for the mutant vector
 *
 * **Example Usage:**
 *
 * ```cpp
 * // Create RandBest1 with λ=0.5 (balanced)
 * auto strategy = std::make_unique<RandBest1>(0.5);
 *
 * // Generate mutant vector
 * std::vector<double> mutant = strategy->mutate(
 *     population,     // Current population
 *     3,              // Target individual index
 *     0.8,            // F = 0.8
 *     bestSolution,   // Best individual found
 *     rng,
 *     -1.0, 1.0       // Domain bounds
 * );
 *
 * // Use mutant for crossover and selection
 *
 * // For adaptive λ:
 * // auto strategy = std::make_unique<RandBest1>(0.4); // Start exploratory
 * // lambda_value = std::min(lambda_value + 0.01, 0.9); // Increase toward exploitative
 * ```
 *
 * **Typical Performance Profile (with λ=0.8):**
 *
 * - **Rosenbrock function:** Excellent (balanced approach works well)
 * - **Rastrigin function:** Good (better than Best1, weaker than Rand1)
 * - **Sphere function:** Excellent (λ bias accelerates convergence)
 * - **Schwefel function:** Very Good (λ balances deception with improvement)
 * - **CEC Benchmark:** Good to Excellent (depends on λ tuning)
 *
 * **Historical Context:**
 *
 * DE/rand-to-best/1 was introduced to provide a principled middle ground between
 * exploration-focused and exploitation-focused strategies. It's been widely adopted
 * in adaptive DE variants and is considered one of the most practical default
 * strategies for unknown problems.
 *
 * @see Mutation for interface documentation
 * @see Rand1 for pure exploration
 * @see Best1 for pure exploitation
 * @see Best2 for best-biased with higher variation
 */
class RandBest1 : public Mutation {
private:
    /// Interpolation parameter controlling exploration-exploitation balance
    /// Range: [0, 1], where 0 = Rand1 (pure exploration), 1 = Best1 (pure exploitation)
    double lambda_;


public:
    /**
     * @brief Constructs a RandBest1 mutation strategy with specified interpolation.
     *
     * **Parameter Selection Guidance:**
     *
     * - **λ = 0.3-0.5:** Highly exploratory, robust to multimodality
     *   - Good default for unknown problems
     *   - Recommended for highly multimodal landscapes
     *
     * - **λ = 0.5-0.7:** Balanced exploration-exploitation
     *   - Practical default for most problems
     *   - Good compromise between speed and robustness
     *
     * - **λ = 0.7-0.9:** More exploitative, faster convergence
     *   - Good for smooth or unimodal problems
     *   - Faster convergence but higher premature convergence risk
     *
     * - **λ = 0.8 (typical):** Slightly exploitative, practical default
     *   - Reasonable balance in most situations
     *   - Starting point for tuning toward problem specifics
     *
     * @param[in] lambda Interpolation parameter in range [0, 1].
     *                   0 = Rand1 (exploration), 1 = Best1 (exploitation).
     *                   Typical values: 0.5-0.8. Default often 0.8.
     *
     * @invariant 0 <= lambda <= 1 (not enforced, but expected by caller)
     *
     * **Example:**
     * ```cpp
     * auto exploratory = std::make_unique<RandBest1>(0.4);  // Explore-biased
     * auto balanced = std::make_unique<RandBest1>(0.5);     // Balanced
     * auto exploitative = std::make_unique<RandBest1>(0.8); // Exploit-biased (default)
     * ```
     */
    explicit RandBest1(double lambda)
            : lambda_(lambda) {}


    /**
     * @brief Generates a mutant vector using DE/rand-to-best/1 strategy.
     *
     * Applies the formula: mutant = current + λ*(best - current) + F*(r1 - r2)
     * where r1 and r2 are randomly selected, and λ controls exploration-exploitation.
     *
     * @param[in] population Current population of solutions
     * @param[in] targetIndex Index of the target individual
     * @param[in] F Differential weight; typical range 0.5-1.2
     * @param[in] bestVector The best solution found so far
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

        // Get population subset        
        const auto& xi  = population[targetIndex];
        const auto& r1  = population[subset[0]];
        const auto& r2  = population[subset[1]];

        std::vector<double> mutated(xi.size());

        for (int i = 0; i < mutated.size(); i++) {
            mutated[i] =
                xi[i]
                + lambda_ * (bestVector[i] - xi[i])
                + F * (r1[i] - r2[i]);

            mutated[i] = std::clamp(mutated[i], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif 

/**
 * @file Rand2.h
 * @author Alex Buckley
 * @brief DE/rand/2 mutation strategy implementation.
 * @ingroup Mutation
 *
 * Implements the DE/rand/2 mutation operator, which applies two weighted
 * difference vectors to a random base individual.
 */


#ifndef RAND2_H
#define RAND2_H

#include "Optimizer/DifferentialEvolution/Mutation/Mutation.h"

#include <algorithm>


/**
 * @class Rand2
 * @ingroup Mutation
 * @brief Implements the DE/rand/2 mutation strategy.
 *
 * **Strategy: DE/rand/2 (Random-2)**
 *
 * An extremely exploratory mutation strategy that applies two independent
 * difference vectors to a random base. This creates maximum variation and
 * is most robust to premature convergence at the cost of slow optimization.
 *
 * **Formula:**
 *
 * @f[
 * v_i(t) = x_{r0}(t) + F \cdot (x_{r1}(t) - x_{r2}(t)) + F \cdot (x_{r3}(t) - x_{r4}(t))
 * @f]
 *
 * Where:
 * - v_i: Mutant vector for target i
 * - x_r0, x_r1, x_r2, x_r3, x_r4: Five randomly selected distinct individuals
 * - F: Differential weight controlling step size (applied to both differences)
 *
 * **Algorithm:**
 *
 * 1. Select 5 random distinct individuals from population (excluding target)
 * 2. For each dimension j:
 *    - mutant[j] = r0[j] + F * (r1[j] - r2[j]) + F * (r3[j] - r4[j])
 *    - Clamp mutant[j] to [lowerBound, upperBound]
 * 3. Return the mutant vector
 *
 * **Characteristics:**
 *
 * **Exploration:** Extreme
 * - Two independent difference vectors provide maximum variation
 * - Random base ensures no bias toward good regions
 * - Creates very diverse mutants
 *
 * **Exploitation:** Minimal
 * - No guidance toward known good solutions
 * - Essentially random walk with directional information
 * - Requires many generations for refinement
 *
 * **Robustness:** Outstanding
 * - Extremely difficult to cause premature convergence
 * - Excellent for deceptive and multimodal landscapes
 * - May explore useless regions extensively
 *
 * **Convergence Speed:** Very slow
 * - Requires many more evaluations than Rand1
 * - Much larger search steps may miss good regions
 * - Best suited for unlimited computational budget
 *
 * **Population Size Requirements:**
 *
 * - **Minimum:** 6 × dimension
 *   - Needs at least 5 random individuals + target
 *   - Requires significantly larger population than Rand1/Best1
 *
 * - **Recommended:** 20-30 × dimension
 *   - Large populations help manage the high variation
 *   - Small populations may not sustain diversity with large mutations
 *
 * **Parameter Recommendations:**
 *
 * - **F (Differential Weight):**
 *   - Smaller F (0.4-0.7): Reduces massive step sizes, more manageable variation
 *   - Larger F (0.7-1.0): Even larger steps, extreme exploration
 *   - Typical: 0.5-0.8 (smaller than Rand1 due to two differences)
 *   - Important: Two differences mean total step can be 2F, so keep F moderate
 *
 * - **CR (Crossover Rate):**
 *   - Lower CR (0.3-0.5): Helps reduce excessive variation
 *   - Higher CR (0.7-0.9): Can amplify the large mutations
 *   - Typical: 0.5-0.8
 *   - Note: Lower CR often better with Rand2 to stabilize mutations
 *
 * - **Generations:**
 *   - Typically 2-5× more than Best1
 *   - Needs time to accumulate improvements
 *   - May require early stopping if no improvement
 *
 * **Comparison with Other Strategies:**
 *
 * | Strategy | Base | Differences | Exploration | Convergence |
 * |----------|------|-------------|-------------|-------------|
 * | Rand1    | Random | 1         | Very High   | Very Slow   |
 * | Best1    | Best   | 1         | Very Low    | Very Fast   |
 * | Rand2    | Random | 2         | Extreme     | Extremely Slow |
 * | Best2    | Best   | 2         | Moderate    | Moderate    |
 * | RandBest1| Hybrid | 1         | Moderate    | Moderate    |
 *
 * **Use Cases:**
 *
 * - Highly deceptive optimization landscapes
 * - Extremely multimodal problems with many similar-quality local optima
 * - When finding any good solution is more important than finding the best
 * - Unbounded computational resources
 * - Baseline robustness testing of problems
 * - Comparison studies measuring strategy differences
 * - Problems where false gradients lead other strategies astray
 *
 * **Avoid When:**
 *
 * - Time is limited (convergence is very slow)
 * - Problem has clear structure (too much variation is wasteful)
 * - Population size is constrained (needs larger populations)
 * - Moderate exploration with better convergence is preferred
 *
 * **Computational Considerations:**
 *
 * **Cost Factors:**
 * - Requires 5 individuals per mutation (vs 3 for Rand1, 2 for Best1)
 * - Very large mutations may be less effective on large-scale problems
 * - Needs larger population to maintain diversity
 *
 * **When It Pays Off:**
 * - Problem has few evaluations due to extreme complexity
 * - Need guaranteed robustness over any other consideration
 * - Problem structure is completely unknown
 *
 * **Implementation Details:**
 *
 * The implementation uses two independent difference vectors:
 * 1. Call getSubset() to randomly select 5 individuals
 * 2. Compute first difference: F * (r1 - r2)
 * 3. Compute second difference: F * (r3 - r4)
 * 4. Sum both with random base r0
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
 * // Create Rand2 strategy
 * auto strategy = std::make_unique<Rand2>();
 *
 * // Generate mutant vector (typically with larger population)
 * std::vector<double> mutant = strategy->mutate(
 *     population,     // Needs larger population (20-30x dimension)
 *     12,             // Target individual index
 *     0.6,            // F = 0.6 (often smaller with Rand2)
 *     bestSolution,   // (unused in Rand2)
 *     rng,
 *     -1.0, 1.0       // Domain bounds
 * );
 *
 * // Use mutant for crossover and selection
 * ```
 *
 * **Typical Performance Profile:**
 *
 * - **Rosenbrock function:** Poor (overly exploratory for simple landscape)
 * - **Rastrigin function:** Excellent (multimodal, finds multiple optima)
 * - **Sphere function:** Poor (overkill exploration)
 * - **Schwefel function:** Excellent (deceptive landscape benefits from robustness)
 * - **CEC Benchmark (multimodal):** Good (if budget allows)
 *
 * **Historical Context:**
 *
 * DE/rand/2 provides the theoretical maximum exploration within the DE framework.
 * While rarely the best choice for a specific problem, it serves as a robustness
 * baseline and is valuable for problems with unknown structure.
 *
 * @see Mutation for interface documentation
 * @see Rand1 for balanced exploration
 * @see Best2 for controlled exploration with best-bias
 * @see RandBest1 for practical robustness with better convergence
 */

class Rand2 : public Mutation {
public:
    /**
     * @brief Generates a mutant vector using DE/rand/2 strategy.
     *
     * Applies the formula: mutant = r0 + F * (r1 - r2) + F * (r3 - r4)
     * where r0, r1, r2, r3, r4 are five randomly selected distinct individuals.
     *
     * @param[in] population Current population of solutions
     * @param[in] targetIndex Index of the target individual (excluded from selection)
     * @param[in] F Differential weight; typical range 0.4-0.8
     * @param[in] bestVector Best solution (unused by Rand2, but required by interface)
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
        std::vector<size_t> subset = getSubset(population.size(), 5, targetIndex, mt);

        // Get population subset        
        const std::vector<double>& r1 = population[subset[0]];
        const std::vector<double>& r2 = population[subset[1]];
        const std::vector<double>& r3 = population[subset[2]];
        const std::vector<double>& r4 = population[subset[3]];
        const std::vector<double>& r5 = population[subset[4]];

        std::vector<double> mutated(r5.size());

        for(int j = 0; j < mutated.size(); j++) {
            mutated[j] = r5[j] 
                + F * (r1[j] - r2[j]) 
                + F * (r3[j] - r4[j]);

            mutated[j] = std::clamp(mutated[j], lowerBound, upperBound);
        }

        return mutated;
    }
};

#endif

/**
 * @file BinCrossover.h
 * @author Alex Buckley
 * @brief Binomial (uniform) crossover strategy implementation.
 * @ingroup Crossover
 *
 * Implements the binomial crossover operator, which independently selects
 * each parameter to inherit from the mutant with uniform probability.
 */


#ifndef BIN_CROSSOVER_H
#define BIN_CROSSOVER_H

#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"

#include <cstddef>


/**
 * @class BinCrossover
 * @ingroup Crossover
 * @brief Implements binomial (uniform) crossover strategy.
 *
 * **Strategy: Binomial Crossover (DE/uniform/1)**
 *
 * The most commonly used crossover strategy. Creates trial vector by independently
 * selecting each parameter to either inherit from mutant or retain from target.
 * Each parameter makes an independent random decision.
 *
 * **Algorithm:**
 *
 * 1. Select one random index jrand (guarantees at least one parameter from mutant)
 * 2. For each parameter i in target:
 *    - If i == jrand OR random() < CR: inherit from mutant
 *    - Else: retain from target
 * 3. Return via in-place modification of target
 *
 * **Pseudocode:**
 *
 * ```
 * jrand = random_index(0, dimension-1)
 * for i = 0 to dimension-1:
 *     if (i == jrand) or (random() < CR):
 *         target[i] = mutant[i]
 * ```
 *
 * **Characteristics:**
 *
 * **Independence:** Each parameter independently chosen
 * - Provides uniform mixing across all dimensions
 * - No spatial correlation between inherited parameters
 * - Symmetric treatment of all parameters
 *
 * **Guarantee:** At least one parameter from mutant
 * - jrand ensures trial ≠ target (guarantees diversity)
 * - Prevents waste of mutant generation
 *
 * **Probability Distribution:** Uniform
 * - Parameter inheritance follows binomial distribution
 * - Expected number of inherited parameters ≈ CR × dimension
 *
 * **Exploration:** Moderate to High
 * - Higher CR → more aggressive exploration
 * - Lower CR → more conservative exploitation
 * - Symmetric mixing encourages exploration in all directions
 *
 * **Typical Behavior:**
 * - CR = 0.3: Few parameters from mutant, mostly target
 * - CR = 0.5: Half parameters from mutant, half from target
 * - CR = 0.9: Most parameters from mutant, few from target
 *
 * **Advantages:**
 *
 * 1. **Simplicity:** Straightforward to understand and implement
 * 2. **Robustness:** Works well on diverse problem types
 * 3. **Default Choice:** Most commonly used crossover in DE literature
 * 4. **Fast:** O(dimension) complexity, minimal overhead
 * 5. **Flexible:** Works with any mutation strategy
 *
 * **Disadvantages:**
 *
 * 1. **No Spatial Structure:** Doesn't exploit problem structure
 * 2. **Independent Decisions:** Can lead to disruptive mixing
 * 3. **Limited for Permutations:** Less effective for permutation problems
 *
 * **When to Use:**
 *
 * - General-purpose optimization (default choice)
 * - Problems with no specific spatial structure
 * - Continuous optimization
 * - Unknown problem characteristics
 * - Need fast, robust crossover
 * - Combining with any mutation strategy
 *
 * **Parameter Recommendations:**
 *
 * **Crossover Rate (CR):**
 * - **CR = 0.1-0.3:** Conservative mixing, good with Best1 mutation
 * - **CR = 0.5-0.7:** Balanced mixing, practical default
 * - **CR = 0.8-0.95:** Aggressive mixing, good with Rand1 mutation
 * - **CR = 1.0:** All parameters from mutant (full replacement)
 * - **Adaptive CR:** Increase CR over time (jDE style) for convergence
 *
 * **Synergy with Mutations:**
 * - **With Rand1:** Use higher CR (0.8-0.95) for exploration
 * - **With Best1:** Use lower CR (0.3-0.7) to stabilize around best
 * - **With RandBest1:** Use medium CR (0.7-0.9) for balance
 * - **With Rand2/Best2:** Use lower CR due to high mutation variation
 *
 * **Implementation Details:**
 *
 * The implementation uses:
 * 1. `mt.genrand_int32() % target.size()` for random index
 * 2. `mt.genrand_real1() < cr` for probability check
 * 3. In-place modification of target
 * 4. Loop over all dimensions
 *
 * **Complexity:**
 *
 * Time: O(dimension) - must examine each parameter
 * Space: O(1) - modifies target in-place
 *
 * **Example Usage:**
 *
 * ```cpp
 * // Create binomial crossover
 * auto crossover = std::make_unique<BinCrossover>();
 *
 * // Apply crossover (modifies target in-place)
 * std::vector<double> target = population[5];  // Current solution
 * std::vector<double> mutant = mutation->mutate(...);  // Mutation result
 * crossover->crossover(target, mutant, 0.9, rng);
 * // Now target is the trial vector
 * ```
 *
 * **Typical Performance Profile:**
 *
 * - **Rosenbrock function:** Excellent (default choice)
 * - **Rastrigin function:** Good (general purpose)
 * - **Sphere function:** Excellent (simple landscape)
 * - **Schwefel function:** Good (deceptive landscape)
 * - **CEC Benchmark:** Good (practical choice)
 *
 * **Historical Context:**
 *
 * Binomial crossover was introduced in the original DE paper by Storn and Price (1997)
 * and remains the most widely used crossover operator. Its simplicity, robustness,
 * and effectiveness across problem types have made it the default choice in most
 * DE implementations.
 *
 * **Comparison with Exponential Crossover:**
 *
 * | Aspect | Binomial | Exponential |
 * |--------|----------|------------|
 * | **Selection** | Independent each parameter | Contiguous block |
 * | **Spatial Bias** | None | Weak (sequential) |
 * | **Exploration** | Uniform | Structured |
 * | **Implementation** | O(dimension) loop | O(inherited count) loop |
 * | **Default** | Yes (most common) | No (specialized) |
 * | **Robustness** | Excellent | Good |
 * | **Best For** | General purpose | Ordered/structured problems |
 *
 * @see Crossover for interface documentation
 * @see ExpCrossover for continuous/exponential alternative
 */
class BinCrossover : public Crossover {
public:
    /**
     * @brief Performs binomial crossover between target and mutant vectors.
     *
     * Independently selects each parameter to inherit from mutant with
     * probability CR. Guarantees at least one parameter from mutant via jrand.
     *
     * @param[in,out] target Target vector, modified in-place to become trial
     * @param[in] mutant Mutant vector as crossover source
     * @param[in] cr Crossover rate; typical range 0.1-0.95
     * @param[in,out] mt Random number generator
     *
     * @post target is modified to be the trial vector
     * @post target[jrand] always came from mutant
     * @post target[i] came from mutant with probability CR for i != jrand
     */
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) override {
        size_t jrand = mt.genrand_int32() % target.size();

        for(size_t i = 0; i < target.size(); i++) {
            if (i == jrand || mt.genrand_real1() < cr)
                target[i] = mutant[i];
        }
    }
};



#endif

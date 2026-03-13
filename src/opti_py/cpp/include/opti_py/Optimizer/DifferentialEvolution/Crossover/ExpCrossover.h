/**
 * @file ExpCrossover.h
 * @author Alex Buckley
 * @brief Exponential (continuous block) crossover strategy implementation.
 * @ingroup Crossover
 *
 * Implements the exponential crossover operator, which inherits parameters
 * in contiguous blocks from the mutant, creating structured variation.
 */


#ifndef EXP_CROSSOVER_H
#define EXP_CROSSOVER_H


#include "Optimizer/DifferentialEvolution/Crossover/Crossover.h"

#include <cstddef>


/**
 * @class ExpCrossover
 * @ingroup Crossover
 * @brief Implements exponential (continuous block) crossover strategy.
 *
 * **Strategy: Exponential Crossover**
 *
 * Inherits parameters from the mutant in contiguous blocks rather than
 * independent selections. Creates a structured, sequential inheritance pattern
 * that can be beneficial for problems with spatial correlations.
 *
 * **Algorithm:**
 *
 * 1. Select one random starting index jrand
 * 2. Starting from jrand, inherit parameters sequentially
 * 3. Continue inheriting while: random() < CR AND haven't inherited all dimensions
 * 4. Wrap around dimensions using modulo (circular)
 * 5. Return via in-place modification of target
 *
 * **Pseudocode:**
 *
 * ```
 * jrand = random_index(0, dimension-1)
 * left = 0
 * do:
 *     idx = (jrand + left) % dimension
 *     target[idx] = mutant[idx]
 *     left++
 * while (random() < CR AND left < dimension)
 * ```
 *
 * **Characteristics:**
 *
 * **Block Inheritance:** Parameters inherited sequentially
 * - Creates contiguous blocks of inherited parameters
 * - Preserves spatial structure from mutant
 * - Can exploit ordered problem structure
 *
 * **Circular Wrapping:** Wraps around at dimension boundaries
 * - Uses modulo arithmetic for wraparound
 * - Treats problem as circular/periodic
 * - Fair treatment of all dimensions
 *
 * **Continuous Probability:** CR controls block length distribution
 * - Block length follows geometric distribution with parameter CR
 * - Expected block length ≈ 1 / (1 - CR)
 * - Higher CR → longer blocks → fewer transitions
 * - Lower CR → shorter blocks → more transitions
 *
 * **Exploration:** Moderate, depends on CR and block structure
 * - CR = 0.3: Very short blocks, many transitions
 * - CR = 0.5: Medium blocks, balanced structure
 * - CR = 0.9: Long blocks, few transitions
 *
 * **Block Length Distribution:**
 *
 * The probability of inheriting exactly k parameters follows:
 * P(k) = (1 - CR)^(k-1) × CR
 *
 * Examples:
 * - CR = 0.5: P(block of 1) = 0.5, P(block of 2) = 0.25, P(block of 3) = 0.125, ...
 * - CR = 0.8: P(block of 1) = 0.2, P(block of 2) = 0.16, P(block of 3) = 0.128, ...
 *
 * **Advantages:**
 *
 * 1. **Structure Preservation:** Maintains spatial correlations
 * 2. **Problem-Specific:** Good for ordered/sequential parameters
 * 3. **Linked Inheritance:** Related parameters inherited together
 * 4. **Lower Disruption:** Fewer independent changes
 * 5. **Variable Block Sizes:** Geometric distribution provides variation
 *
 * **Disadvantages:**
 *
 * 1. **Bias:** Block starting position creates preference
 * 2. **Less General:** May perform worse on unstructured problems
 * 3. **Order-Dependent:** Sensitive to parameter ordering
 * 4. **Complex Behavior:** Block structure less intuitive than binomial
 * 5. **Slower Selection:** Loop continues until CR condition fails
 *
 * **When to Use:**
 *
 * - Problems with spatial/sequential parameter structure
 * - Linked parameters that should co-evolve
 * - Gene grouping problems (epistasis)
 * - Problems where parameter order is meaningful
 * - When binomial crossover causes disruptive mixing
 * - Neural network weight optimization
 * - Sensor/coordinate problems (x, y, z groupings)
 * - Avoiding mutation-like disruption from independent selection
 *
 * **Avoid When:**
 *
 * - Problem is unstructured (use binomial)
 * - Parameters are independent (use binomial)
 * - Parameter ordering is arbitrary
 * - Need maximum generality (use binomial)
 * - Problem structure is unknown
 *
 * **Parameter Recommendations:**
 *
 * **Crossover Rate (CR):**
 * - **CR = 0.1-0.3:** Very short blocks, high disruption
 * - **CR = 0.5-0.7:** Medium blocks, balanced structure
 * - **CR = 0.8-0.95:** Long blocks, less disruption
 * - **Adaptive CR:** Can adapt block length over optimization
 * - **Problem-Dependent:** Choose based on parameter correlations
 *
 * **Synergy with Mutations:**
 * - **With Rand1:** Use medium-high CR (0.7-0.9) for structured exploration
 * - **With Best1:** Use lower CR (0.3-0.7) to maintain structure around best
 * - **With RandBest1:** Use medium CR (0.6-0.85) for balanced structure
 * - **With Rand2/Best2:** Use lower CR due to high mutation variation
 *
 * **Implementation Details:**
 *
 * The implementation uses:
 * 1. `mt.genrand_int32() % target.size()` for random starting index
 * 2. `(jrand + left) % target.size()` for circular indexing
 * 3. `mt.genrand_real1() < cr` for continuation probability
 * 4. In-place modification of target
 * 5. Do-while loop for block inheritance
 *
 * **Complexity:**
 *
 * Time: O(inherited_count) where inherited_count is variable (0 to dimension)
 *       Expected: O(CR × dimension)
 * Space: O(1) - modifies target in-place
 *
 * **Example Usage:**
 *
 * ```cpp
 * // Create exponential crossover
 * auto crossover = std::make_unique<ExpCrossover>();
 *
 * // Apply crossover (modifies target in-place)
 * std::vector<double> target = population[5];  // Current solution
 * std::vector<double> mutant = mutation->mutate(...);  // Mutation result
 * crossover->crossover(target, mutant, 0.8, rng);
 * // Now target is the trial vector with contiguous blocks from mutant
 * ```
 *
 * **Example Inheritance Patterns:**
 *
 * With 8-dimensional problem, CR=0.7, jrand=2:
 * - Iteration 1: idx=2, inherit (random < 0.7), left=1
 * - Iteration 2: idx=3, inherit (random < 0.7), left=2
 * - Iteration 3: idx=4, inherit (random < 0.7), left=3
 * - Iteration 4: idx=5, DON'T inherit (random ≥ 0.7), stop
 * Result: target[2:5] from mutant, rest retained
 *
 * **Typical Performance Profile:**
 *
 * - **Rosenbrock function:** Good (structure-aware)
 * - **Rastrigin function:** Moderate (less general)
 * - **Sphere function:** Good (simple landscape)
 * - **Schwefel function:** Moderate (deceptive)
 * - **Ordered Parameter Problem:** Excellent (best choice)
 * - **General CEC Benchmark:** Moderate (less robust than binomial)
 *
 * **Historical Context:**
 *
 * Exponential crossover was introduced in the original DE paper by Storn and Price (1997)
 * as an alternative to binomial crossover. While less commonly used than binomial,
 * it provides value for problems with inherent parameter structure.
 *
 * **Comparison with Binomial Crossover:**
 *
 * | Aspect | Binomial | Exponential |
 * |--------|----------|------------|
 * | **Selection** | Independent each parameter | Contiguous block |
 * | **Spatial Bias** | None | Weak (sequential) |
 * | **Exploration** | Uniform | Structured |
 * | **Block Length** | Poisson-like (each parameter) | Geometric (contiguous) |
 * | **Implementation** | O(dimension) loop | O(inherited count) loop |
 * | **Default** | Yes (most common) | No (specialized) |
 * | **Robustness** | Excellent (general) | Good (specialized) |
 * | **Best For** | General purpose | Ordered/structured problems |
 *
 * **Tuning Guide:**
 *
 * If standard binomial isn't working well:
 * 1. Check if parameters have spatial/sequential meaning
 * 2. If yes, try exponential with CR = 0.5-0.8
 * 3. Adjust CR based on expected block size
 * 4. Monitor if blocks help or hurt fitness progress
 * 5. Consider adaptive CR that evolves block sizes
 *
 * @see Crossover for interface documentation
 * @see BinCrossover for independent/uniform alternative
 */
class ExpCrossover : public Crossover {
public:
    /**
     * @brief Performs exponential crossover between target and mutant vectors.
     *
     * Inherits parameters in contiguous blocks from mutant starting at random
     * index jrand. Block length determined by CR probability.
     *
     * @param[in,out] target Target vector, modified in-place to become trial
     * @param[in] mutant Mutant vector as crossover source
     * @param[in] cr Crossover rate controlling block length; typical range 0.1-0.95
     * @param[in,out] mt Random number generator
     *
     * @post target is modified to be the trial vector
     * @post target contains one or more contiguous blocks from mutant
     * @post block(s) start at jrand and wrap around circularly
     * @post expected number of inherited parameters ≈ cr × size when cr < 1.0
     */
    void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) override {
        size_t jrand = mt.genrand_int32() % target.size();
        size_t left = 0;

        do {
            size_t idx = (jrand + left) % target.size();
            target[idx] = mutant[idx];
            left++;
        } while (mt.genrand_real1() < cr && left < target.size());
    }
};


#endif

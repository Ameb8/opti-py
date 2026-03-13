/**
 * @file Crossover.h
 * @author Alex Buckley
 * @brief Base class for Differential Evolution crossover strategies.
 * @defgroup Crossover DE Crossover Strategies
 * @ingroup DifferentialEvolution
 *
 * Defines the abstract Crossover base class and provides the common interface
 * for all DE crossover strategy implementations.
 */


#ifndef CROSSOVER_H
#define CROSSOVER_H


#include <vector>

#include "External/mt.h"


/**
 * @class Crossover
 * @ingroup Crossover
 * @brief Abstract base class for Differential Evolution crossover strategies.
 *
 * Crossover defines the interface for all DE crossover operators. Each concrete
 * crossover strategy (BinCrossover, ExpCrossover) implements the crossover() method
 * to recombine a target vector and mutant vector, creating a trial vector for
 * evaluation.
 *
 * **Crossover Role in DE:**
 *
 * In Differential Evolution, crossover blends the target and mutant vectors:
 * 1. Start with target vector from current population
 * 2. Mix in parameters from mutant vector with probability CR
 * 3. Create trial vector for fitness evaluation
 * 4. Selection step compares trial vs target
 *
 * **Purpose of Crossover:**
 *
 * - **Diversity Control:** Crossover rate (CR) controls how aggressively
 *   new variation enters the population
 * - **Inheritance:** Selects which parameters to inherit from mutant
 * - **Exploration-Exploitation Balance:** Works with mutation (F) to control
 *   overall search behavior
 *
 * **Crossover Rate (CR) Semantics:**
 *
 * CR is a probability in range [0, 1]:
 * - **CR = 0.0:** No crossover, trial = target (no change)
 * - **CR = 0.5:** Each parameter has 50% chance to come from mutant
 * - **CR = 1.0:** All parameters from mutant (full replacement)
 * - **Typical:** 0.5-0.9 depending on problem
 *
 * **Crossover vs Mutation:**
 *
 * The pair (F, CR) controls search behavior:
 * - **Mutation (F):** Determines magnitude of variation in mutant
 * - **Crossover (CR):** Determines how much variation enters population
 * - **Interaction:** High F + High CR = aggressive exploration
 *           Low F + Low CR = conservative exploitation
 *
 * **Strategy Comparison:**
 *
 * Two main strategies:
 * - **Binomial (Uniform):** Each parameter independently inherited
 * - **Exponential (Continuous):** Parameters inherited in contiguous blocks
 *
 * @see BinCrossover for binomial (independent) crossover
 * @see ExpCrossover for exponential (continuous) crossover
 */
class Crossover {
public:
    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~Crossover() = default;

    /**
     * @brief Performs crossover between target and mutant vectors.
     *
     * Pure virtual method that each concrete crossover strategy implements
     * to recombine a target vector and mutant vector. The target vector is
     * modified in-place to become a trial vector.
     *
     * **Algorithm Pattern:**
     *
     * All crossover strategies follow this general workflow:
     * 1. Select at least one index jrand (ensures at least one parameter from mutant)
     * 2. For each parameter in target:
     *    - With probability CR, inherit from mutant
     *    - Otherwise, keep current value from target
     * 3. Parameter at jrand is always inherited (guarantees diversity)
     * 4. Return via in-place modification of target
     *
     * **In-Place Modification:**
     *
     * The target vector is modified directly, replacing its contents with
     * the trial vector. This is efficient (no copy) and standard in DE.
     *
     * **Guarantees:**
     *
     * - Target vector is always modified (in-place)
     * - Trial vector dimension equals target and mutant dimension
     * - At least one parameter comes from mutant (jrand index)
     * - All other parameters inherit with probability CR
     *
     * **Parameters:**
     *
     * @param[in,out] target The target (parent) vector from current population.
     *                       Modified in-place to become the trial vector.
     *                       Size: equals mutant.size() and problem dimension.
     *
     * @param[in] mutant The mutant vector produced by mutation strategy.
     *                   Used as source for inherited parameters.
     *                   Size: equals target.size().
     *
     * @param[in] cr Crossover rate (probability of inheritance).
     *              Range: [0, 1].
     *              - cr = 0: Very few parameters from mutant
     *              - cr = 0.5: About half parameters from mutant
     *              - cr = 1.0: All parameters from mutant
     *              Typical values: 0.5-0.9.
     *
     * @param[in,out] mt Mersenne Twister random number generator.
     *                   Passed by reference; state is modified during execution.
     *                   Used to randomly select parameters to inherit.
     *
     * **Strategy-Specific Behavior:**
     *
     * Different concrete strategies implement different inheritance patterns:
     * - **Binomial:** Each parameter independently chosen with probability CR
     * - **Exponential:** Parameters inherited in contiguous blocks
     *
     * **Postconditions:**
     *
     * After crossover completes:
     * - target vector contains trial vector (modified in-place)
     * - At least one parameter came from mutant (at index jrand)
     * - Other parameters inherited with probability CR
     * - target.size() unchanged
     *
     * **Usage in DE:**
     *
     * ```cpp
     * std::vector<double> target = population[i];  // Current solution
     * std::vector<double> mutant = mutation->mutate(...);  // Mutation step
     * crossover->crossover(target, mutant, 0.9, rng);  // Crossover step
     * // Now target is the trial vector, evaluate fitness
     * double trial_fitness = problem.evaluate(target);
     * ```
     *
     * **Complexity:**
     *
     * Time: O(dimension) - must examine each parameter
     * Space: O(1) - modifies target in-place
     *
     * @see BinCrossover for independent parameter selection
     * @see ExpCrossover for contiguous block selection
     */
    virtual void crossover(
        std::vector<double>& target,
        const std::vector<double>& mutant,
        double cr,
        MersenneTwister& mt
    ) = 0;
};

#endif

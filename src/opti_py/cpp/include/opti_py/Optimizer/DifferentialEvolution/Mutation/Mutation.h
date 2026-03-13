/**
 * @file Mutation.h
 * @author Alex Buckley
 * @brief Base class for Differential Evolution mutation strategies.
 * @defgroup Mutation DE Mutation Strategies
 * @ingroup DifferentialEvolution
 *
 * Defines the abstract Mutation base class and provides the common interface
 * and utilities for all DE mutation strategy implementations.
 */


#ifndef MUTATION_H
#define MUTATION_H

#include <vector>
#include <cstddef>

#include "External/mt.h"


/**
 * @class Mutation
 * @ingroup Mutation
 * @brief Abstract base class for Differential Evolution mutation strategies.
 *
 * Mutation defines the interface and shared utilities for all DE mutation operators.
 * Each concrete mutation strategy (Rand1, Best1, Rand2, Best2, RandBest1) implements
 * the mutate() method to generate a mutant vector from a population based on a
 * selected mutation formula.
 *
 * **Mutation Role in DE:**
 *
 * In Differential Evolution, mutation is the primary source of variation:
 * 1. Select random individuals from the population
 * 2. Apply a weighted combination of differences to create new variation
 * 3. The mutation formula determines which individuals and differences are used
 *
 * **Mutation Formula Notation:**
 *
 * Standard DE mutation formulas follow the pattern: DE/base/N, where:
 * - **base:** The base vector used as starting point (either random or best)
 * - **N:** The number of difference vectors applied (1 or 2)
 *
 * Common formulas:
 * - DE/rand/1: mutant = r0 + F*(r1 - r2)
 * - DE/rand/2: mutant = r0 + F*(r1 - r2) + F*(r3 - r4)
 * - DE/best/1: mutant = best + F*(r1 - r2)
 * - DE/best/2: mutant = best + F*(r1 - r2) + F*(r3 - r4)
 * - DE/rand-to-best/1: mutant = x + λ*(best - x) + F*(r1 - r2)
 *
 * **Scaling Factor F:**
 *
 * The differential weight F (typically in range 0.0 to 2.0) controls the amplitude
 * of variation. Key considerations:
 * - F ≈ 0.5-0.8: Conservative exploration, good for fine-tuning
 * - F ≈ 0.8-1.5: Moderate exploration-exploitation balance
 * - F > 1.5: Aggressive exploration, may miss good regions
 *
 * **Individual Selection:**
 *
 * The getSubset() helper selects random distinct individuals from the population
 * (excluding the target individual). This ensures the mutant is derived from
 * different genetic material than the target.
 *
 * @note All mutation strategies inherit from this base class and override mutate().
 * @note The public interface consists solely of the virtual mutate() method.
 * @see Rand1, Rand2, Best1, Best2, RandBest1 for concrete implementations
 */
class Mutation {
public:
    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~Mutation() = default;


    /**
     * @brief Generates a mutant vector using the mutation strategy.
     *
     * Pure virtual method that each concrete mutation strategy implements
     * to produce a mutant vector. The mutant combines selected population
     * individuals according to the strategy's formula.
     *
     * **Algorithm Pattern:**
     *
     * All mutation strategies follow this general workflow:
     * 1. Select N random distinct individuals from the population (via getSubset())
     * 2. Compute a weighted combination of these individuals
     * 3. Clamp the result to the domain bounds
     * 4. Return the mutant vector
     *
     * **Parameters:**
     *
     * @param[in] population The complete population of candidate solutions.
     *                       Each element is a vector of dimension equal to the
     *                       optimization problem. Size: popSize × dimension.
     *
     * @param[in] targetIndex Index of the target individual in the population.
     *                        This individual is excluded from random selection
     *                        (cannot use its own genetic material for mutation).
     *                        Range: [0, popSize-1].
     *
     * @param[in] F Differential weight (mutation scaling factor).
     *             Controls the amplitude of variation in the mutation formula.
     *             Typical range: (0, 2], common values: 0.5-1.0.
     *             - Larger F: more aggressive exploration
     *             - Smaller F: more conservative fine-tuning
     *
     * @param[in] bestVector The best (lowest fitness) individual found so far.
     *                       Used in "best" strategies (Best1, Best2, RandBest1).
     *                       For "rand" strategies, may be ignored.
     *                       Dimension: same as population elements.
     *
     * @param[in,out] mt Mersenne Twister random number generator.
     *                   Used to randomly select individuals and generate variations.
     *                   Passed by reference; state is modified during execution.
     *
     * @param[in] lowerBound Lower bound of the feasible domain.
     *                        All dimensions share this uniform bound.
     *                        Used to clamp mutant values to valid range.
     *
     * @param[in] upperBound Upper bound of the feasible domain.
     *                        All dimensions share this uniform bound.
     *                        Used to clamp mutant values to valid range.
     *
     * **Return Value:**
     *
     * @return A new mutant vector with dimension matching the population.
     *         All values are within [lowerBound, upperBound] due to clamping.
     *         Size: equal to population[0].size().
     *
     * **Postconditions:**
     *
     * The returned mutant vector satisfies:
     * - Dimension matches population dimension
     * - All elements are within [lowerBound, upperBound]
     * - Is derived from 2-5 random population members (depends on strategy)
     *
     * **Strategy-Specific Behavior:**
     *
     * Different concrete strategies implement different formulas:
     * - Rand1: Uses 3 random individuals; more exploratory
     * - Best1: Uses best individual as base; more exploitative
     * - Rand2: Uses 5 random individuals; more variation, slower
     * - Best2: Uses best with 4 additional individuals; balanced
     * - RandBest1: Interpolates between best and random; hybrid approach
     *
     * **Complexity:**
     *
     * Time: O(dimension) - linear in the problem dimension
     * Space: O(dimension) - for the returned mutant vector
     *
     * **Usage Example:**
     *
     * ```cpp
     * std::unique_ptr<Mutation> strategy = std::make_unique<Best1>();
     * std::vector<double> mutant = strategy->mutate(
     *     population,
     *     targetIdx,
     *     0.8,           // F
     *     bestSolution,
     *     rng,
     *     -1.0, 1.0      // bounds
     * );
     * ```
     *
     * @see getSubset() for the random selection mechanism
     */
    virtual std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        size_t targetIndex,
        double F,
        const std::vector<double>& bestVector,
        MersenneTwister& mt,
        double lowerBound,
        double upperBound
    ) = 0;


protected:
    /** @name Shared Utilities */
    ///@{
 
    
    /**
     * @brief Selects a random subset of population indices, excluding a source index.
     *
     * This is a shared utility used by all mutation strategies to randomly select
     * distinct individuals from the population. The selection uses a partial Fisher-Yates
     * shuffle for efficiency and guarantees no duplicates.
     *
     * **Algorithm:**
     *
     * 1. Create a list of all valid indices [0, populationSize-1] except source
     * 2. Perform first subsetSize iterations of Fisher-Yates shuffle:
     *    - For each position i, randomly swap with position j in range [i, size)
     * 3. Return the first subsetSize elements (which are now randomized)
     *
     * **Complexity:**
     *
     * Time: O(populationSize + subsetSize) for setup and shuffle
     * Space: O(populationSize) for temporary index vector
     *
     * **Parameters:**
     *
     * @param[in] populationSize Total number of individuals in the population.
     *                            Range: subsetSize + 1 to infinity.
     *
     * @param[in] subsetSize Number of distinct indices to select.
     *                        Range: [1, populationSize - 1].
     *                        Must be less than populationSize (to exclude source).
     *
     * @param[in] source Index to exclude from the selection.
     *                   This individual is not included in the returned subset.
     *                   Range: [0, populationSize - 1].
     *
     * @param[in,out] mt Mersenne Twister random number generator.
     *                   Passed by reference; state is modified.
     *                   Used to generate random swap positions.
     *
     * **Return Value:**
     *
     * @return A vector of subsetSize distinct indices from [0, populationSize-1],
     *         excluding source. All elements are unique.
     *         Size: exactly subsetSize.
     *
     * **Guarantees:**
     *
     * - All returned indices are distinct (no duplicates)
     * - No returned index equals source
     * - All indices are in range [0, populationSize - 1]
     * - The selection is uniformly random (all subsets equally likely)
     *
     * **Usage in Mutation Strategies:**
     *
     * Mutation formulas require 2-5 distinct random individuals:
     * ```cpp
     * // For DE/rand/1: need 3 individuals
     * std::vector<size_t> r = getSubset(popSize, 3, targetIndex, mt);
     * // r[0], r[1], r[2] are three distinct random individuals
     *
     * // For DE/rand/2: need 5 individuals
     * std::vector<size_t> r = getSubset(popSize, 5, targetIndex, mt);
     * ```
     *
     * **Example:**
     *
     * ```cpp
     * // Select 3 random individuals from a population of 20, excluding individual 5
     * auto indices = getSubset(20, 3, 5, rng);
     * // Possible result: [8, 15, 2] (three random distinct values != 5)
     * ```
     *
     * **Preconditions:**
     *
     * - populationSize >= subsetSize + 1 (at least room for subset + source)
     * - source < populationSize
     * - subsetSize > 0
     *
     * **Implementation Note:**
     *
     * The partial Fisher-Yates algorithm is used instead of full shuffle or
     * repeated sampling for O(populationSize + subsetSize) complexity without
     * the risk of duplicate rejection sampling.
     */
    std::vector<size_t> getSubset(
        size_t populationSize,
        size_t subsetSize,
        size_t source,
        MersenneTwister& mt
    ) {
        std::vector<size_t> indices(populationSize - 1);
        size_t idx = 0;

        // Prepare vector of all valid indices except source
        for(size_t i = 0; i < populationSize; i++) {
            if(i != source) {
                indices[idx] = i;
                ++idx;
            }
        }

        // Partial Fisher-Yates shuffle to select subsetSize random indices
        for(size_t i = 0; i < subsetSize; i++) {
            int j = i + (mt.genrand_int32() % (indices.size() - i));
            std::swap(indices[i], indices[j]);
        }

        // Return only the first 'subsetSize' indices
        indices.resize(subsetSize);
        return indices;
    }
};

#endif
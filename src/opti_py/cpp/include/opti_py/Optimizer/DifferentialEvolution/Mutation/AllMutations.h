/**
 * @file AllMutations.h
 * @author Alex Buckley
 * @brief Convenience header aggregating all DE mutation strategy implementations.
 * @ingroup Mutation
 *
 * This header provides a single include point for all Differential Evolution
 * mutation strategy classes. Use this to include all strategies at once, or
 * include individual strategy headers for selective use.
 *
 * **Included Strategies:**
 *
 * - Rand1: DE/rand/1 (pure exploration)
 * - Rand2: DE/rand/2 (maximum exploration)
 * - Best1: DE/best/1 (pure exploitation)
 * - Best2: DE/best/2 (balanced)
 * - RandBest1: DE/rand-to-best/1 (tunable hybrid)
 *
 * **Usage:**
 *
 * Include all strategies:
 * ```cpp
 * #include "Optimizer/DifferentialEvolution/Mutation/AllMutations.h"
 * ```
 *
 * Or include selectively:
 * ```cpp
 * #include "Optimizer/DifferentialEvolution/Mutation/Best1.h"
 * #include "Optimizer/DifferentialEvolution/Mutation/Rand1.h"
 * ```
 *
 * **Note:** The base Mutation class is not automatically included.
 * Include "Mutation.h" separately if needed for the abstract interface.
 *
 * @see Mutation for the base class interface
 * @see Rand1, Rand2, Best1, Best2, RandBest1 for individual strategy documentation
 */



#ifndef ALL_MUTATIONS_H
#define ALL_MUTATIONS_H


#include "Optimizer/DifferentialEvolution/Mutation/Best1.h"
#include "Optimizer/DifferentialEvolution/Mutation/Best2.h"
#include "Optimizer/DifferentialEvolution/Mutation/Rand1.h"
#include "Optimizer/DifferentialEvolution/Mutation/Rand2.h"
#include "Optimizer/DifferentialEvolution/Mutation/RandBest1.h"

#endif
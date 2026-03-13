/**
 * @file AllCrossovers.h
 * @author Alex Buckley
 * @brief Convenience header aggregating all DE crossover strategy implementations.
 * @ingroup Crossover
 *
 * This header provides a single include point for all Differential Evolution
 * crossover strategy classes. Use this to include all strategies at once, or
 * include individual strategy headers for selective use.
 *
 * **Included Strategies:**
 *
 * - BinCrossover: Binomial (uniform, independent) crossover
 * - ExpCrossover: Exponential (continuous block) crossover
 *
 * **Usage:**
 *
 * Include all strategies:
 * ```cpp
 * #include "Optimizer/DifferentialEvolution/Crossover/AllCrossovers.h"
 * ```
 *
 * Or include selectively:
 * ```cpp
 * #include "Optimizer/DifferentialEvolution/Crossover/BinCrossover.h"
 * #include "Optimizer/DifferentialEvolution/Crossover/ExpCrossover.h"
 * ```
 *
 * **Note:** The base Crossover class is not automatically included.
 * Include "Crossover.h" separately if needed for the abstract interface.
 *
 * @see Crossover for the base class interface
 * @see BinCrossover for binomial (default) crossover documentation
 * @see ExpCrossover for exponential (structured) crossover documentation
 */



#ifndef ALL_CROSSOVERS_H
#define ALL_CROSSOVERS_H

#include "Optimizer/DifferentialEvolution/Crossover/BinCrossover.h"
#include "Optimizer/DifferentialEvolution/Crossover/ExpCrossover.h"

#endif
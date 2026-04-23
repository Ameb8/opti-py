# OptiPy and SciPy Performance Comparison

In order to gauge the performance of OptiPy, a comparison between the two was performed. In this benchmark, solution quality (solution fitness), varience, and wall-clock execution time was analyzed. Both programs optimized the following programs with Differential Evolution:

- Schwefel

- Rastrigin

- Rosenbrock

- Griewangk

- Ackley 1

- Ackley 2

- De Jong 1

## Experiment Methodology

Both solvers used identical hyperparameters: mutation factor F = 0.8, crossover rate CR = 0.5, population size 15×D (150 for D = 10), and a maximum of 500 generations. The DE variant was rand/1/bin in both cases. To ensure a fair evaluation budget, SciPy's popsize parameter was set to 15 (per-dimension) so its internal population matched OptiPy's exactly, and tol=0 was set to prevent early termination. SciPy's optional L-BFGS-B polishing step was disabled (polish=False) to keep both solvers as pure DE.

Each function was run 10 times with independent seeds (42–51), and mean fitness, standard deviation, best fitness, and mean wall-clock time were recorded across runs. The primary comparison metric is mean fitness at budget exhaustion, which captures typical rather than lucky performance.

One deliberate asymmetry exists between the two solvers: SciPy was configured to use all available CPU cores for parallelism, while OptiPy used its default OpenMP-based threading. Informal testing showed SciPy to achieve better performance when utilizing all cores, thus, this was used in the benchmark. Exact experiment setup can be viewed [in the benchmark directory of this repository](https://github.com/Ameb8/opti-py/tree/master/benchmark).

## Results

The benchmark results show a clear and consistent pattern: OptiPy prioritizes computational efficiency while SciPy emphasizes solution precision. Across all tested functions, OptiPy completes optimization runs roughly an order of magnitude faster, typically finishing in well under half a second, whereas SciPy requires several seconds per run. This gap is substantial and highlights one of OptiPy’s core strengths—its ability to deliver rapid results, making it well-suited for scenarios where evaluation time is critical, such as iterative experimentation, large-scale sweeps, or real-time applications. Below shows the performance results of the benchmark, while exact results can be [found here](https://github.com/Ameb8/opti-py/blob/master/results/benchmark_results.csv).

![OptiPy and SciPy Performance Comparison](https://github.com/Ameb8/opti-py/blob/master/results/time_comparison.png)

In terms of solution quality, SciPy consistently converges closer to known global optima, particularly on difficult landscapes like Rosenbrock and Schwefel. However, this improvement in accuracy comes with significantly higher computational cost. OptiPy’s results, while less optimal numerically, remain within a reasonable range for many practical purposes, especially considering the speed advantage. On functions like Rastrigin and Griewangk, OptiPy demonstrates that it can still capture the general structure of the search space and produce meaningful approximations in a fraction of the time.

![OptiPy and SciPy Fitness Comparison](https://github.com/Ameb8/opti-py/blob/master/results/fitness_comparison.png)

Another notable aspect is variability. OptiPy exhibits higher standard deviation across runs, suggesting a more exploratory search behavior. While this can appear as instability in strict benchmarking terms, it also indicates that the algorithm is less tightly constrained and may be better positioned to explore diverse regions of the search space under limited budgets. This characteristic can be advantageous in early-stage optimization or when dealing with noisy or poorly understood objective functions.

![OptiPy and SciPy Variability Comparison](https://github.com/Ameb8/opti-py/blob/master/results/std_comparison.png)

Overall, the results position OptiPy as a fast and flexible DE implementation that trades off some degree of convergence precision for substantial gains in runtime performance. Rather than directly competing with highly tuned libraries like SciPy on final accuracy, OptiPy distinguishes itself by enabling rapid optimization cycles. This makes it particularly valuable as a lightweight, high-speed alternative in workflows where turnaround time matters as much as, or more than, absolute optimality.
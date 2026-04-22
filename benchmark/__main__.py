"""
benchmark/__main__.py
-----------------
Orchestrates the OptiPy vs SciPy benchmark run.

Usage
-----
    # From the repo root (so `benchmark` is importable as a package):
    python -m benchmark.main

    # Or run directly (adds parent dir to sys.path automatically):
    python benchmark/main.py

Output
------
  results/
    benchmark_results.csv
    fitness_comparison.png
    std_comparison.png
    time_comparison.png
    fitness_scatter.png
"""

from __future__ import annotations

import sys
from pathlib import Path

# Allow `python benchmark/main.py` without installing the package.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from benchmark import config
from benchmark.functions import BENCHMARKS
from benchmark.runners import run_optipy, run_scipy
from benchmark.reporting import print_summary, save_csv, save_plots
from benchmark.stats import FunctionResult


def run_benchmark(out_dir: Path = Path("results")) -> list[FunctionResult]:
    """Execute all benchmark functions across all seeds and return results."""
    results: list[FunctionResult] = []

    total = len(BENCHMARKS) * config.N_RUNS
    done  = 0

    for spec in BENCHMARKS:
        fr = FunctionResult(name=spec.name)

        for seed in config.SEEDS:
            done += 1
            print(f"  [{done:>{len(str(total))}}/{total}] "
                  f"{spec.name:<20}  seed={seed}", flush=True)

            f_op, t_op = run_optipy(spec.problem_id, seed)
            f_sc, t_sc = run_scipy(spec.scipy_fn, spec.bounds, seed)

            fr.record_optipy(f_op, t_op)
            fr.record_scipy(f_sc, t_sc)

        fr.finalise()
        results.append(fr)

    return results


def main() -> None:
    out_dir = Path("results")

    print("\nStarting benchmark …\n")
    results = run_benchmark(out_dir)

    print_summary(results)
    save_csv(results, out_dir)
    save_plots(results, out_dir)

    print("\nDone. All artefacts written to ./results/\n")


if __name__ == "__main__":
    main()
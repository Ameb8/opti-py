"""
benchmark/runners.py
--------------------
Thin wrappers around OptiPy and SciPy's DE solvers that apply the shared
hyperparameters from config and return a uniform (fitness, elapsed) tuple.
"""

from __future__ import annotations

import time
from typing import TYPE_CHECKING

import numpy as np
from scipy.optimize import differential_evolution as _scipy_de

from opti_py import Problem, optimize_de

from . import config
from .functions import ScipyObjective, Bounds1D

if TYPE_CHECKING:
    pass  # for forward-reference annotations only


# ── Result type ───────────────────────────────────────────────────────────────
type RunResult = tuple[float, float]  # (best_fitness, wall_clock_seconds)


# ── Runners ───────────────────────────────────────────────────────────────────

def run_optipy(problem_id: Problem, seed: int) -> RunResult:
    """Run OptiPy DE on *problem_id* and return (best_fitness, elapsed_s)."""
    t0 = time.perf_counter()
    result = optimize_de(
        problem_id=problem_id,
        f=config.F,
        cr=config.CR,
        max_generations=config.MAX_GENS,
        seed=seed,
        mutation_strategy="rand1",
        crossover_strategy="bin",
    )
    elapsed = time.perf_counter() - t0
    return float(result.fitness), elapsed


def run_scipy(
    func: ScipyObjective,
    bounds_1d: Bounds1D,
    seed: int,
) -> RunResult:
    """Run SciPy DE on *func* with the shared budget and return (best_fitness, elapsed_s).

    Key fairness settings
    ---------------------
    ``tol=0``        — disable convergence-based early stopping
    ``polish=False`` — no post-hoc L-BFGS-B refinement
    ``popsize``      — set so total population equals ``config.POP_SIZE``
    """
    bounds = [bounds_1d] * config.DIM
    t0 = time.perf_counter()
    result = _scipy_de(
        func,
        bounds,
        strategy="rand1bin",
        maxiter=config.MAX_GENS,
        popsize=config.SCIPY_POPSIZE,
        mutation=config.F,
        recombination=config.CR,
        seed=seed,
        tol=0,
        polish=False,
        init="latinhypercube",
        workers=-1,
    )
    elapsed = time.perf_counter() - t0
    return float(result.fun), elapsed
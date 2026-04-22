"""
benchmark/functions.py
----------------------
SciPy-compatible benchmark function definitions and a registry that maps
human-readable names to the OptiPy Problem enum, the SciPy callable,
and the per-dimension search bounds.

All function signatures accept a NumPy array and return a scalar float,
matching the interface expected by scipy.optimize.differential_evolution.
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Callable

import numpy as np

# OptiPy's Problem enum — imported here so callers never touch opti_py directly.
from opti_py import Problem


# ── Type aliases ──────────────────────────────────────────────────────────────
ScipyObjective = Callable[[np.ndarray], float]
Bounds1D       = tuple[float, float]


# ── Benchmark registry entry ──────────────────────────────────────────────────
@dataclass(frozen=True)
class BenchmarkSpec:
    name:       str
    problem_id: Problem
    scipy_fn:   ScipyObjective
    bounds:     Bounds1D


# ── Function implementations ──────────────────────────────────────────────────

def schwefel(x: np.ndarray) -> float:
    n = len(x)
    return float(418.9829 * n - np.sum(x * np.sin(np.sqrt(np.abs(x)))))


def rastrigin(x: np.ndarray) -> float:
    n = len(x)
    return float(10 * n + np.sum(x**2 - 10 * np.cos(2 * math.pi * x)))


def rosenbrock(x: np.ndarray) -> float:
    return float(np.sum(100.0 * (x[1:] - x[:-1] ** 2) ** 2 + (1 - x[:-1]) ** 2))


def griewangk(x: np.ndarray) -> float:
    i = np.arange(1, len(x) + 1)
    return float(1.0 + np.sum(x**2 / 4000) - np.prod(np.cos(x / np.sqrt(i))))


def ackley_one(x: np.ndarray) -> float:
    n = len(x)
    return float(
        -0.2 * np.sqrt((1 / n) * np.sum(x**2))
        + 3 * (np.e - np.exp((1 / n) * np.sum(np.cos(2 * math.pi * x))))
    )


def ackley_two(x: np.ndarray) -> float:
    return float(
        20
        - 20 * np.exp(-0.2 * np.sqrt(np.sum(x**2) / len(x)))
        + math.e
        - np.exp(np.sum(np.cos(2 * math.pi * x)) / len(x))
    )


def de_jong_one(x: np.ndarray) -> float:
    return float(np.sum(x**2))


# ── Registry ──────────────────────────────────────────────────────────────────
BENCHMARKS: list[BenchmarkSpec] = [
    BenchmarkSpec("Schwefel",    Problem.SCHWEFEL,   schwefel,    (-500.0, 500.0)),
    BenchmarkSpec("Rastrigin",   Problem.RASTRIGIN,  rastrigin,   (-5.12,  5.12)),
    BenchmarkSpec("Rosenbrock",  Problem.ROSENBROCK, rosenbrock,  (-2.048, 2.048)),
    BenchmarkSpec("Griewangk",   Problem.GRIEWANGK,  griewangk,   (-600.0, 600.0)),
    BenchmarkSpec("Ackley One",  Problem.ACKLEY_ONE, ackley_one,  (-32.0,  32.0)),
    BenchmarkSpec("Ackley Two",  Problem.ACKLEY_TWO, ackley_two,  (-32.0,  32.0)),
    BenchmarkSpec("De Jong One", Problem.DEJONG_ONE, de_jong_one, (-5.12,  5.12)),
]
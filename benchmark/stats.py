"""
benchmark/stats.py
------------------
Lightweight dataclasses to hold per-solver and per-function results,
plus a helper that computes summary statistics from a list of run values.
"""

from __future__ import annotations

import statistics
from dataclasses import dataclass, field


@dataclass
class SolverStats:
    """Aggregate statistics for one solver on one benchmark function."""
    mean_fitness: float
    std_fitness:  float
    best_fitness: float
    mean_time:    float  # seconds

    @property
    def fitness_winner_over(self, other: "SolverStats") -> bool:
        return self.mean_fitness <= other.mean_fitness

    @property
    def speed_winner_over(self, other: "SolverStats") -> bool:
        return self.mean_time <= other.mean_time


@dataclass
class FunctionResult:
    """Holds raw per-run data and derived stats for both solvers on one function."""
    name: str

    optipy_fitnesses: list[float] = field(default_factory=list)
    optipy_times:     list[float] = field(default_factory=list)
    scipy_fitnesses:  list[float] = field(default_factory=list)
    scipy_times:      list[float] = field(default_factory=list)

    # Populated by finalise()
    optipy: SolverStats | None = field(default=None, repr=False)
    scipy:  SolverStats | None = field(default=None, repr=False)

    def record_optipy(self, fitness: float, elapsed: float) -> None:
        self.optipy_fitnesses.append(fitness)
        self.optipy_times.append(elapsed)

    def record_scipy(self, fitness: float, elapsed: float) -> None:
        self.scipy_fitnesses.append(fitness)
        self.scipy_times.append(elapsed)

    def finalise(self) -> None:
        """Compute SolverStats from the accumulated raw lists."""
        self.optipy = _summarize(self.optipy_fitnesses, self.optipy_times)
        self.scipy  = _summarize(self.scipy_fitnesses,  self.scipy_times)

    @property
    def optipy_wins_fitness(self) -> bool:
        assert self.optipy and self.scipy
        return self.optipy.mean_fitness <= self.scipy.mean_fitness

    @property
    def optipy_wins_speed(self) -> bool:
        assert self.optipy and self.scipy
        return self.optipy.mean_time <= self.scipy.mean_time


# ── Internal helper ───────────────────────────────────────────────────────────

def _summarize(fitnesses: list[float], times: list[float]) -> SolverStats:
    return SolverStats(
        mean_fitness=statistics.mean(fitnesses),
        std_fitness=statistics.stdev(fitnesses) if len(fitnesses) > 1 else 0.0,
        best_fitness=min(fitnesses),
        mean_time=statistics.mean(times),
    )
"""
benchmark/config.py
-------------------
Shared hyperparameters for the OptiPy vs SciPy benchmark.
All tuning knobs live here — change once, propagate everywhere.
"""

# ── Problem dimensionality ────────────────────────────────────────────────────
DIM: int = 10

# ── DE hyperparameters ────────────────────────────────────────────────────────
F: float  = 0.8   # mutation factor
CR: float = 0.5   # crossover rate

# ── Population & generation budget ───────────────────────────────────────────
# Standard convention: popsize = 15 × D
POP_SIZE: int = 15 * DIM
MAX_GENS: int = 500

# SciPy's `popsize` param is per-dimension (total pop = popsize * N).
# Setting it to POP_SIZE // DIM keeps total population identical to OptiPy.
SCIPY_POPSIZE: int = POP_SIZE // DIM   # = 15

# ── Reproducibility ───────────────────────────────────────────────────────────
N_RUNS: int       = 10
BASE_SEED: int    = 42
SEEDS: list[int]  = list(range(BASE_SEED, BASE_SEED + N_RUNS))
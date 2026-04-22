"""
benchmark/reporting.py
----------------------
Handles all output artifacts produced after a benchmark run:

  - Console summary table (preserves the original print output)
  - CSV export  → results/benchmark_results.csv
  - Plots        → results/
      ├── fitness_comparison.png   (grouped bar, mean fitness per function)
      ├── std_comparison.png       (grouped bar, std fitness)
      ├── time_comparison.png      (grouped bar, wall-clock time)
      └── fitness_scatter.png      (scatter: OptiPy fitness vs SciPy fitness)
"""

from __future__ import annotations

import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

from . import config
from .stats import FunctionResult

# ── Output directory ──────────────────────────────────────────────────────────
RESULTS_DIR = Path("results")

# ── Palette ───────────────────────────────────────────────────────────────────
_OPTIPY_COLOR = "#2563EB"   # blue
_SCIPY_COLOR  = "#DC2626"   # red
_ALPHA        = 0.85


# ── Public API ────────────────────────────────────────────────────────────────

def print_summary(results: list[FunctionResult]) -> None:
    """Reproduce the original console output in a clean tabular format."""
    header = (
        f"\n{'═'*78}\n"
        f"  OptiPy vs SciPy — Differential Evolution Benchmark\n"
        f"  Dim={config.DIM}  PopSize={config.POP_SIZE}  MaxGens={config.MAX_GENS}  "
        f"F={config.F}  CR={config.CR}  Runs={config.N_RUNS}\n"
        f"  Strategy: rand1/bin  |  Seeds: {config.SEEDS[0]}–{config.SEEDS[-1]}\n"
        f"{'═'*78}"
    )
    print(header, flush=True)

    for r in results:
        assert r.optipy and r.scipy
        op, sc = r.optipy, r.scipy
        fw = "OptiPy" if r.optipy_wins_fitness else "SciPy "
        sw = "OptiPy" if r.optipy_wins_speed   else "SciPy "

        print(f"\n{'─'*78}")
        print(f"  {r.name}")
        print(f"{'─'*78}")
        print(f"  {'Metric':<24} {'OptiPy':>14} {'SciPy':>14}  {'Winner':>8}")
        print(f"  {'─'*24} {'─'*14} {'─'*14}  {'─'*8}")
        print(f"  {'Mean Fitness':<24} {op.mean_fitness:>14.4e} {sc.mean_fitness:>14.4e}  {fw:>8}")
        print(f"  {'Std Fitness':<24} {op.std_fitness:>14.4e}  {sc.std_fitness:>14.4e}")
        print(f"  {'Best Fitness':<24} {op.best_fitness:>14.4e} {sc.best_fitness:>14.4e}")
        print(f"  {'Mean Wall Time (s)':<24} {op.mean_time:>14.4f} {sc.mean_time:>14.4f}  {sw:>8}")

    # Win totals
    n = len(results)
    op_fit = sum(1 for r in results if r.optipy_wins_fitness)
    op_spd = sum(1 for r in results if r.optipy_wins_speed)

    print(f"\n\n{'═'*78}")
    print("  SUMMARY")
    print(f"{'═'*78}")
    print(f"  {'Function':<20} {'OptiPy Mean':>14} {'SciPy Mean':>14}  "
          f"{'Fitness':>10}  {'Speed':>10}")
    print(f"  {'─'*20} {'─'*14} {'─'*14}  {'─'*10}  {'─'*10}")

    for r in results:
        assert r.optipy and r.scipy
        fw = "OptiPy ✓" if r.optipy_wins_fitness else "SciPy  ✓"
        sw = "OptiPy ✓" if r.optipy_wins_speed   else "SciPy  ✓"
        print(f"  {r.name:<20} {r.optipy.mean_fitness:>14.4e} "
              f"{r.scipy.mean_fitness:>14.4e}  {fw:>10}  {sw:>10}")

    print(f"\n  Fitness wins — OptiPy: {op_fit}/{n}   SciPy: {n - op_fit}/{n}")
    print(f"  Speed wins   — OptiPy: {op_spd}/{n}   SciPy: {n - op_spd}/{n}")
    print(f"{'═'*78}\n")
    sys.stdout.flush()


def save_csv(results: list[FunctionResult], out_dir: Path = RESULTS_DIR) -> Path:
    """Write per-function, per-solver statistics to a CSV file.

    Columns
    -------
    function, solver, mean_fitness, std_fitness, best_fitness, mean_time_s
    """
    out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = out_dir / "benchmark_results.csv"

    fields = ["function", "solver", "mean_fitness", "std_fitness",
              "best_fitness", "mean_time_s"]

    with csv_path.open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=fields)
        writer.writeheader()
        for r in results:
            assert r.optipy and r.scipy
            for solver_name, stats in (("OptiPy", r.optipy), ("SciPy", r.scipy)):
                writer.writerow({
                    "function":     r.name,
                    "solver":       solver_name,
                    "mean_fitness": stats.mean_fitness,
                    "std_fitness":  stats.std_fitness,
                    "best_fitness": stats.best_fitness,
                    "mean_time_s":  stats.mean_time,
                })

    print(f"[reporting] CSV saved → {csv_path}")
    return csv_path


def save_plots(results: list[FunctionResult], out_dir: Path = RESULTS_DIR) -> list[Path]:
    """Generate and save all benchmark plots. Returns a list of saved paths."""
    out_dir.mkdir(parents=True, exist_ok=True)
    saved: list[Path] = []

    saved.append(_plot_grouped_bars(
        results, out_dir,
        metric_fn=lambda r: (r.optipy.mean_fitness, r.scipy.mean_fitness),  # type: ignore[union-attr]
        err_fn=lambda r: (r.optipy.std_fitness, r.scipy.std_fitness),        # type: ignore[union-attr]
        title="Mean Fitness — OptiPy vs SciPy",
        ylabel="Mean fitness (lower is better)",
        filename="fitness_comparison.png",
        log_scale=True,
    ))

    saved.append(_plot_grouped_bars(
        results, out_dir,
        metric_fn=lambda r: (r.optipy.std_fitness, r.scipy.std_fitness),    # type: ignore[union-attr]
        err_fn=None,
        title="Fitness Std Dev — OptiPy vs SciPy",
        ylabel="Std deviation of fitness",
        filename="std_comparison.png",
        log_scale=True,
    ))

    saved.append(_plot_grouped_bars(
        results, out_dir,
        metric_fn=lambda r: (r.optipy.mean_time, r.scipy.mean_time),        # type: ignore[union-attr]
        err_fn=None,
        title="Mean Wall-Clock Time — OptiPy vs SciPy",
        ylabel="Time (s)",
        filename="time_comparison.png",
        log_scale=False,
    ))

    saved.append(_plot_scatter(results, out_dir))

    return saved


# ── Private plot helpers ──────────────────────────────────────────────────────

def _plot_grouped_bars(
    results: list[FunctionResult],
    out_dir: Path,
    *,
    metric_fn,
    err_fn,
    title: str,
    ylabel: str,
    filename: str,
    log_scale: bool,
) -> Path:
    names           = [r.name for r in results]
    optipy_vals     = []
    scipy_vals      = []
    optipy_errs     = []
    scipy_errs      = []

    for r in results:
        op_v, sc_v = metric_fn(r)
        optipy_vals.append(op_v)
        scipy_vals.append(sc_v)
        if err_fn:
            op_e, sc_e = err_fn(r)
            optipy_errs.append(op_e)
            scipy_errs.append(sc_e)

    x      = np.arange(len(names))
    width  = 0.35
    fig, ax = plt.subplots(figsize=(12, 5))

    kw_err = dict(capsize=4, elinewidth=1.2, capthick=1.2)
    ax.bar(
        x - width / 2, optipy_vals, width,
        label="OptiPy", color=_OPTIPY_COLOR, alpha=_ALPHA,
            yerr=(optipy_errs if err_fn else None),
            error_kw=kw_err if err_fn else None,
    )
    ax.bar(
        x + width / 2, scipy_vals, width,
        label="SciPy",  color=_SCIPY_COLOR,  alpha=_ALPHA,
            yerr=(optipy_errs if err_fn else None),
            error_kw=kw_err if err_fn else None,
    )

    if log_scale:
        ax.set_yscale("log")
        ax.yaxis.set_major_formatter(ticker.LogFormatterSciNotation())

    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=20, ha="right", fontsize=10)
    ax.set_ylabel(ylabel, fontsize=11)
    ax.set_title(title, fontsize=13, fontweight="bold", pad=12)
    ax.legend(fontsize=11)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()

    path = out_dir / filename
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"[reporting] Plot saved  → {path}")
    return path


def _plot_scatter(results: list[FunctionResult], out_dir: Path) -> Path:
    """Scatter plot: log(OptiPy mean fitness) vs log(SciPy mean fitness).

    Points above the diagonal → SciPy wins fitness.
    Points below the diagonal → OptiPy wins fitness.
    """
    op_vals = [r.optipy.mean_fitness for r in results]  # type: ignore[union-attr]
    sc_vals = [r.scipy.mean_fitness  for r in results]  # type: ignore[union-attr]
    names   = [r.name for r in results]

    fig, ax = plt.subplots(figsize=(6, 6))

    for op, sc, name in zip(op_vals, sc_vals, names):
        color = _OPTIPY_COLOR if op <= sc else _SCIPY_COLOR
        ax.scatter(op, sc, color=color, s=90, zorder=3, alpha=0.9)
        ax.annotate(
            name, (op, sc),
            textcoords="offset points", xytext=(6, 4),
            fontsize=8, color="#374151",
        )

    # Diagonal y = x
    lim_min = min(min(op_vals), min(sc_vals)) * 0.5
    lim_max = max(max(op_vals), max(sc_vals)) * 2.0
    ax.plot([lim_min, lim_max], [lim_min, lim_max],
            "k--", linewidth=1, alpha=0.5, label="y = x  (tie)")

    ax.set_xscale("symlog")
    ax.set_yscale("symlog")
    ax.set_xlabel("OptiPy mean fitness", fontsize=11)
    ax.set_ylabel("SciPy mean fitness",  fontsize=11)
    ax.set_title(
        "Mean Fitness Scatter\n(below diagonal = OptiPy wins, above = SciPy wins)",
        fontsize=12, fontweight="bold",
    )
    ax.legend(fontsize=10)
    ax.grid(linestyle="--", alpha=0.35)
    fig.tight_layout()

    path = out_dir / "fitness_scatter.png"
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"[reporting] Plot saved  → {path}")
    return path
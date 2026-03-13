import pandas as pd

from pathlib import Path

from .plot_builder import build_plots


def build_results(data: pd.DataFrame, output_dir: Path, verbose: bool = False) -> None:
    # Create dir to store plots
    plot_dir: Path = output_dir / 'plots'
    plot_dir.mkdir(parents=True, exist_ok=True)

    build_plots(data, plot_dir, verbose)
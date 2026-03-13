import pandas as pd
import numpy as np

from pathlib import Path

from .plot_builder import build_plots
from .tex_builder import generate_tex


def build_results(data: pd.DataFrame, output_dir: Path, verbose: bool = False) -> None:
    # Define paths to output directories
    plot_dir: Path = output_dir / 'plots'
    table_dir: Path = output_dir / 'tables'

    # Ensure output directories exist
    plot_dir.mkdir(parents=True, exist_ok=True)
    table_dir.mkdir(parents=True, exist_ok=True)

    # Generate outputs
    build_plots(data, plot_dir, verbose)
    generate_tex(data, table_dir, True, verbose)
import pandas as pd

from pathlib import Path
import sys

from .run_benchmarks import run_benchmarks
from .result_builder import build_results
from .analyze_data import average_seeds, aggregate_alg


def main() -> None:
    data_dir: Path = Path.cwd()
    output_dir: Path = Path.cwd() / 'results'

    # Read path to data dir
    if len(sys.argv) > 1:
        data_dir = data_dir / sys.argv[1]

    # Ensure data directory exists
    if not data_dir.exists() or not data_dir.is_dir():
        sys.exit(
            f'Invalid Argument: Path must point to directory'
            f'containing test data:\t{str(data_dir)}'
        )

    print(f'\nLoading flow shop problems from {data_dir}\n')

    # Execute all experiments
    results: pd.DataFrame = run_benchmarks(data_dir)
    print(f'\n{len(results)} experiments executed\n\n\n')

    print('\n\n\nRaw Results:\n\n')
    debug_df(results)

    # Compute result statistis
    averaged = average_seeds(results, config_cols=[
        'blocking', 'max_gens', 'pop_size', 'f', 'cr', 
        'seed', 'mutation', 'crossover', 'exp_name', 'optimize_tardiness'
    ])

    print('\n\n\nAveraged Results:\n\n')
    debug_df(averaged)

    compare_alg: pd.DataFrame = aggregate_alg(averaged)
    print('\n\n\nAlgorithm Comparison:\n\n')
    debug_df(compare_alg)


    # Build results
    build_results(compare_alg, output_dir)
    print(f'\nExperiment results stored in {output_dir}\n')


def debug_df(df: pd.DataFrame, name: str = "DataFrame", max_rows: int = 10):

    print(f"\n{'='*60}")
    print(f"DEBUG: {name}")
    print(f"{'='*60}")

    if df is None:
        print("DataFrame is None")
        return

    print(f"Shape: {df.shape}")
    print(f"Columns ({len(df.columns)}): {list(df.columns)}\n")

    print("Dtypes:")
    print(df.dtypes)

    print("\nMissing values:")
    print(df.isnull().sum())

    print(f"\nHead ({max_rows} rows):")
    print(df.head(max_rows))

    print(f"\nTail ({max_rows} rows):")
    print(df.tail(max_rows))

    print(f"{'='*60}\n")

if __name__ == "__main__":
    main()
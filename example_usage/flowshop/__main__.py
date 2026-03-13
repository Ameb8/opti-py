import pandas as pd

import argparse
import sys
from pathlib import Path
from typing import Any

from .load_config import load_param_grid, ParamGridError
from .run_benchmarks import run_benchmarks
from .analyze_data import average_seeds, aggregate_alg
from .result_builder import build_results


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments for data directory, output directory, and config file.
    
    Returns:
        argparse.Namespace: Contains config_file, data_dir, and output_dir as Path objects.
    
    Usage:
        python script.py config.yaml --data ./data --output ./results
        python script.py config.yaml -d ./data -o ./results
    """
    parser = argparse.ArgumentParser(
        description="Process data using configuration."
    )
    
    # Config file ppth
    parser.add_argument(
        "-c", "--config",
        dest="config_file",
        type=Path,
        default=Path.cwd() / "config.toml",
        help="Path to configuration file (default: ./config.toml)"
    )
    
    # Data directory path
    parser.add_argument(
        "-d", "--data",
        dest="data_dir",
        type=Path,
        default=Path.cwd() / "data",
        help="Path to data directory (default: ./data)"
    )
    
    # Results directory path
    parser.add_argument(
        "-o", "--output",
        dest="output_dir",
        type=Path,
        default=Path.cwd() / "results",
        help="Path to output directory (default: ./results)"
    )

    # Verbose output
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    
    # Debug argument
    parser.add_argument(
        "-dbg", "--debug",
        action="store_true",
        help="Enable debug output"
    )

    args = parser.parse_args()
    
    return args


def main() -> None:
    try:
        # Parse command-line arguments
        args = parse_args()

        # Load project config
        exp_config: dict[str, Any] = load_param_grid(args.config_file)

        # Ensure data directory exists
        if not args.data_dir.exists() or not args.data_dir.is_dir():
            sys.exit(
                f'Invalid Argument: Path must point to directory'
                f'containing test data:\t{str(args.data_dir)}'
            )

        print(f'\nLoading flow shop problems from {args.data_dir}\n')

        # Execute all experiments
        results: pd.DataFrame = run_benchmarks(
            exp_config, 
            args.data_dir, 
            args.verbose
        )

        print(f'\n{len(results)} experiments executed\n\n\n')

        if args.debug:
            print('\n\n\nRaw Results:\n\n')
            debug_df(results)

        # Compute result statistics
        averaged = average_seeds(results, config_cols=[
            'blocking', 'max_gens', 'pop_size', 'f', 'cr', 
            'seed', 'mutation', 'crossover', 'exp_name', 'optimize_tardiness'
        ])

        if args.debug:
            print('\n\n\nAveraged Results:\n\n')
            debug_df(averaged)

        compare_alg: pd.DataFrame = aggregate_alg(averaged)
    
        if args.debug:
            print('\n\n\nAlgorithm Comparison:\n\n')
            debug_df(compare_alg)


        # Build results
        build_results(compare_alg, args.output_dir, args.verbose)
        print(f'\nExperiment results stored in {args.output_dir}\n')
    except FileNotFoundError as e:
        sys.exit(f"Config file not found: {e}")
    except ParamGridError as e:
        sys.exit(f"Invalid configuration: {e}")
    except Exception as e:
        sys.exit(f"Unexpected error occurred: {e}")



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
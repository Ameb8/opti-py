import pandas as pd

from pathlib import Path
import sys

from .run_benchmarks import run_benchmarks

def main() -> None:
    data_dir: Path = Path.cwd()

    # Read path to data dir
    if len(sys.argv) > 1:
        data_dir = data_dir / sys.argv[1]

    # Ensure data directory exists
    if not data_dir.exists() or not data_dir.is_dir():
        sys.exit(
            f'Invalid Argument: Path must point to directory'
            f'containing test data:\t{str(data_dir)}'
        )

    run_benchmarks(data_dir)


if __name__ == "__main__":
    main()
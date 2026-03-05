from opti_py import FlowShop, FlowShopResult
import numpy as np
import pandas as pd

from pathlib import Path
import time
from typing import Any


def run_benchmarks(data_dir: Path) -> pd.DataFrame:
    max_benchmark: int = 120 # Number of benchmarks in config
    benchmark_results: list[dict[str, Any]] = []# Stores experiment results

    for test_num in range(max_benchmark): # Iterate benchmark cases
        # Run experiment
        experiment_config: Path = data_dir / f'{test_num + 1}.txt'
        benchmark_results.extend(run_experiment(experiment_config, test_num + 1))

    return pd.DataFrame(benchmark_results)


def exec_experiment(experiment, blocking: bool = False) -> dict[str, Any]:
    # Time and execute NEH heuristic on flowshop problem
    start_time: float = time.perf_counter()
    result: FlowShopResult = experiment.run_neh(blocking=blocking)
    end_time: float = time.perf_counter()

    # Construct result dict from experiment config and results
    result_dict: dict[str, Any] = vars(experiment).copy()
    result_dict.update(vars(result))

    # Add fields for blocking and execution times
    result_dict["blocking"] = blocking
    result_dict["exec_time"] = end_time - start_time

    return result_dict


def run_experiment(experiment_config: Path, exp_num: int) -> list[dict[str, Any]]:
    # Load job execution times per machine from input file
    job_times: np.ndarray = np.loadtxt(
        experiment_config,
        skiprows=1, # Skip num machines and num jobs line
        dtype=np.uint64 
    )

    # Instantiate FlowShop object with experiment config
    experiment: FlowShop = FlowShop(job_times)

    return [ # Return blocking and non-blocking result dicts
        exec_experiment(experiment),
        exec_experiment(experiment, blocking=True)
    ]


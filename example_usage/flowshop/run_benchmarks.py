from opti_py import FlowShop, FlowShopResult, Mutation, Crossover
from tqdm import tqdm
import numpy as np
import pandas as pd


from pathlib import Path
from typing import Any, Iterator
import itertools
import time

from .experiment import Experiment


def count_configs(grid: dict[str, list[Any]]) -> int:
    total = 1
    for v in grid.values():
        total *= len(v)
    return total

def iter_configs(grid: dict[str, list[Any]]) -> Iterator[Experiment]:
    for values in itertools.product(*grid.values()):
        yield Experiment(*values)

def run_benchmarks(param_grid: dict[str, list[Any]], data_dir: Path, verbose: bool = False) -> pd.DataFrame:
    benchmark_results: list[dict[str, Any]] = []# Stores experiment results

    # Collect all data files
    data_files = list(data_dir.glob('*.txt'))

    # Count number of runs
    configs_per_file: int = count_configs(param_grid)
    total_experiments: int = configs_per_file * len(data_files)

    # Create one global progress bar
    pbar = None # No progress bar in verbose mode
    if not verbose:
        pbar = tqdm(total=total_experiments, desc="Benchmarks")


    for data_file in data_files:
        # Display current data file name in progress bar
        if pbar is not None:
            pbar.set_postfix(file=data_file.stem)

        # Load job execution times per machine from input file
        job_times: np.ndarray = np.loadtxt(
            data_file,
            skiprows=1, # Skip num machines and num jobs line (derived from job times size)
            dtype=np.uint64 
        )
        
        # Instantiate FlowShop object job times matrix
        problem: FlowShop = FlowShop(job_times)

        # Run benchmark with all configurations
        for config in iter_configs(param_grid):
            benchmark_results.append(run_experiment(
                problem, 
                config, 
                data_file.stem,
                verbose
            ))

            # Update progress bar
            if pbar is not None:
                pbar.update(1)

    # Close progress bar
    if pbar is not None:
        pbar.close()

    # Return results as dataframe
    return pd.DataFrame(benchmark_results)



def run_experiment(problem: FlowShop, config: Experiment, exp_name: str, verbose: bool = False) -> dict[str, Any]:
    # Start timing
    start_time: float = time.perf_counter()
    
    # Run experiment
    result: FlowShopResult = problem.run_de(
        blocking=config.blocking,
        tardiness=config.optimize_tardiness,
        max_gens=config.max_gens,
        pop_size=config.pop_size,
        f=config.f,
        cr=config.cr,
        seed=config.seed,
        mutation=config.mutation,
        crossover=config.crossover
    )

    # End timing
    end_time: float = time.perf_counter()

    # Construct result dict from experiment config and results
    result_dict: dict[str, Any] = config.to_dict()
    result_dict.update(result.to_dict())
    result_dict.update(problem.to_dict())

    # Add fields for execution time
    result_dict['exec_time'] = end_time - start_time
    result_dict['exp_name'] = exp_name

    if verbose: # Print experiment if in verbose mode
        print(f'Experiment {exp_name}\tDE/{config.mutation.value}/{config.crossover.value}:\t\t{result.makespan}')

    return result_dict
        

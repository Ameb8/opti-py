from opti_py import FlowShop, FlowShopResult, Mutation, Crossover
import numpy as np
import pandas as pd

from pathlib import Path
import itertools
import time
from typing import Any, Iterator

from .experiment import Experiment


param_grid = {
    'blocking': [False, True],
    'optimize_tardiness': [False, True],
    'max_gens': [200],
    'pop_size': [200],
    'f': [0.5],
    'cr': [0.9],
    'seed': [1, 2, 3],
    'mutation': list(Mutation),
    'crossover': list(Crossover)
}

def iter_configs(grid: dict[str, list[Any]]) -> Iterator[Experiment]:
    for values in itertools.product(*grid.values()):
        yield Experiment(*values)

def run_benchmarks(data_dir: Path) -> pd.DataFrame:
    benchmark_results: list[dict[str, Any]] = []# Stores experiment results

    for data_file in data_dir.glob('*.txt'):
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
                data_file.stem
            ))

    # Return results as dataframe
    return pd.DataFrame(benchmark_results)



def run_experiment(problem: FlowShop, config: Experiment, exp_name: str) -> dict[str, Any]:
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

    # Print experiment
    print(f'Experiment {exp_name}\tDE/{config.mutation.value}/{config.crossover.value}: {result.makespan}')


    return result_dict
        

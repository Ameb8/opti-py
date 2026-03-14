# OptiPy

OptiPy is a Python Optimization library designed to handle both the speed and efficiency of C++ code with the usability of Python code. It has support for minimizing standard benchmark functions, as well as optimizing the flow shop problem. The public interface provided by OptiPy is entirely in Python, including support for creating and running Optimizer objects.

OptiPy is designed to provide a simple interface for efficiently calculating optimization problems without imposing too many system and hardware requirements. This is done by building the optimization algorithms in *C++* as well as using *OpenMP* for CPU-based parallelism. 

# Using OptiPy

## Installation

As this package is not yet available on `pip`, source code must be downloaded in order to install it. The full source code for this library can be found [on the project's Github page](https://github.com/Ameb8/opti-py). After downloading the source code, it can be installed with pip in the following manner:

```bash
pip install <path to cloned repository root>
```

after installed with `pip`, it can be used in any directory location as long as you are using the same Python environment in which it was installed. All C++ code is compiled automatically during the installation process.

## Requirements
- **C++20 compatible compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake 3.15+**
- **OpenMP 4.0+**
- **Python 3.11+**


## Running Optimizations

### Optimizing the Flow Shop Problem

#### Creating Flow Shop Problems

OptiPy supports flow shop optimizations, both with and without blocking. the NEH heuristic is used to minimize makespan given a selection of jobs sand machines. Problem specifications are configured by creating an instance of the FlowShop class. Conventionally, this can be done with a NumPy matrix with a dtype of uint64. While other integer types can be used to instantiate a FlowShop object, the class does not support negative or decimal values. 

Each row in the matrix represents a job while each column represents a machine. Thus, each element *[i, j]* of the matrix represents the amount of time job *i* takes to complete on machine *j*. Below is an example of a flow shop problem where the first job takes 3 seconds on all 5 machines, while the second job takes 1 second on the first,2 on the second, etc.

```python
from opti_py import FlowShop
import numpy as np

# Create processing times matrix
processing_times = np.array(
    [
        [3, 3, 3, 3, 3],  # Job 0
        [1, 2, 3, 4, 5],  # Job 1
    ],
    dtype=np.uint64
)

# Instantiate FlowShop problem with 2 jobs and 5 machines
problem: FlowShop = FlowShop(processing_times)
```

A Default constructor taking no arguments can optionally be invoked, however, a numpy array must then be assigned to the jobs field before flow shop calculations can be made. Thus, the following code creates a FlowShop object identical to the one above.

```python
from opti_py import FlowShop
import numpy as np

# Create processing times matrix
processing_times = np.array(
    [
        [3, 3, 3, 3, 3],  # Job 0
        [1, 2, 3, 4, 5],  # Job 1
    ],
    dtype=np.uint64
)

# Instantiate FlowShop problem with default constructor
problem: FlowShop = FlowShop()

# Assign job times matrix after construction
problem.jobs = processing_times

```

Additionally, a FlowShop object can be created with dimensions as arguments, resulting in a zero-initialized job times matrix. Job times can then be set manually by accessing the `jobs` field like a normal NumPy array. Thus, the following code will construct a FlowShop object identical to the previous examples.

```python
from opti_py import FlowShop

# Dimensions for jobs matrix
num_jobs = 2
num_machines = 5

# initialize FlowShop with zeroed-out job times matrix
problem: FlowShop = FlowShop(2, 5)

# Set first job processing times to 3 on all machines
for i in range(5):
    problem.jobs[0][i] = 3

# Set second job processing times from 1 to 5 incrementally
for i in range(5):
    problem.jobs[1][i] = i + 1
```

In addition to direct element modification, the `jobs` field can be reassigned to a new matrix. The `num_jobs` and `num_machines` field will automatically be set upon reassignment. However, it is important to note that when the `jobs` field is reassigned to a new NumPy array, OptiPy cleans up the old jobs array. While this is essential for library performance, it can cause bugs and undefined behavior if not handled carefully. The following code demonstrates an example of undefined behavior, a pattern which must be avoided when using this library.

```python
from opti_py import FlowShop
import numpy as np

# Create jobs matrix inline in FlowShop constructor
problem: FlowShop = FlowShop(
    np.array([[1, 2], [3, 4]])
)

initial_jobs: np.ndarray = problem.jobs

# Access jobs matrix validly
print(f"Valid Jobs Matrix: {initial_jobs}")

# Reassign new jobs matrix to FlowShop object
# Automatically frees memory used for previous jobs matrix
problem.jobs = np.array([[5, 6], [7, 8]])

# --------------------------------------------------------
# UNDEFINED BEHAVIOR
# --------------------------------------------------------
# The initial_jobs array now points to freed memory
# Accessing the initial_jobs variable will cause undefined behavior
print(f"Reference to Freed Memory: {initial_jobs}")
```

##### Properties
 
| Property | Type | Access | Description |
| :--- | :--- | :--- | :--- |
| **jobs** | *np.ndarray[uint64]* (2D) | Read/Write | Processing time matrix where *jobs[i, j]* is the time job *i* takes on machine *j*. Can be assigned a new numpy array; doing so invalidates previous references to the old array. |
| **due_dates** | *np.ndarray[uint64]* (1D) | Read/Write | Due date for each job. Length must equal `num_jobs`. Used in tardiness calculations when optimizing with `tardiness=True`. |
| **num_jobs** | *int* | Read-only | Number of jobs in the problem. |
| **num_machines** | *int* | Read-only | Number of machines in the problem. |

#### Optimizing the Flow Shop Problem

##### Optimizing with NEH Heuristic

Once a FlowShop object has been created, the `run_neh` method can be invoked in order to minimize makespan using the NEH heuristic. Note that the FlowShop object must have a valid NumPy array of positive integer-types in the jobs field. Because this optimization relies solely on the NEH heuristic, it may not always achieve absolutely optimal scheduling. the `run_neh` function takes an optional `blocking` argument, controlling whether or not the problem uses the blocking variation of flow shop. If not included, the optimization defaults to non-blocking. Thus, blocking an non-blocking calculations with the NEH heuristic can be made in the following manner.

```Python
from opti_py import FlowShop, FlowShopResult
import numpy as np

# Create processing times matrix
processing_times = np.array(
    [
        [3, 3, 3, 3, 3],  # Job 0
        [1, 2, 3, 4, 5],  # Job 1
    ]
)

# Instantiate FlowShop problem with 2 jobs and 5 machines
problem: FlowShop = FlowShop(processing_times)

# Make calculation for blocking and non-blocking problem variations
results_non_blocking: FlowShopResult = problem.run_neh()
results_blocking: FlowShopResult = problem.run_neh(blocking=True)
```

The same FlowShop object can be used to conduct any number of optimizations, with or without modifications to the `jobs` field matrix in between runs. Modifications to a FlowShop object will not affect FlowShopResult objects that it has already created. Additionally, NEH can be used to opimize for tardiness instead of makespan. 

##### Optimizing with Differential Evolution

Differential Evolution can utilized to optimize the flow shop problem, either for makespan or tardiness. This can be applied to flow shop problems with or without blocking. After creating a valid FlowShop object (stored as `flow_shop` variable in this example), DE optimization can be performed as follows:
```python

result: FlowShopResult = flow_shop.run_de(
    blocking=False, # Indicates whether problem should use blocking
    tardiness=False, # True if optimizing for tardiness, false to optimize for makespan
    max_gens=100, # Maximum number of DE generations
    pop_size=200, # DE population size
    f=0.8, # DE mutation factor
    cr=0.5, # DE crossover rate
    seed=108664, # Seed used for pseudo-random number generator in DE
    mutation="rand1", # DE mutation type
    crossover="bin" # DE crossover type
)
```

the run_de function can also be invoked with no arguments, as defaults are provided. For example, the exact same experiment as above could be run with the following invocation:

```python
result: FlowShopResult = flow_shop.run_de()
```



#### Flow Shop Results


| Field | Type | Description |
| :--- | :--- | :--- |
| **sequence** | *list[int]* | Order in which jobs are executed. *sequence[i]* represents the index of the job iin the original *FlowShop.jobs* table | 
| **makespan** | *int* | Total amount of time needed to execute all jobs on all machines |
| **completion_times** | *list[list[int]]* | The time from start each job finishes on each machine. *completion_times[i][j]* is the time job *i* finishes execution on machine *j*. Job *i* refers to the job scheduled in the *ith* position, not the *ith* row in original jobs matrix.|
| **tardiness** | *list[int]* | The tardiness of value of each job with the given sequence |
| **fitnesses** | *list[int]* | The best fitness found at each generation. If optimized for tardiness, it will be the sum of each jobs tardiness in the best solution at that iteration. Otherwise, *fitnesses[i]* will be the best makespan found at generation *i*. |



In addition to data fields, the FlowShopResult class offers a `to_dict()` method. When invoked, this method constructs a python dictionary of type *dict[str, Any]*, with Keys exactly matching field names and values matching the field values. As the class has no underlying `__dict__` property like native Python classes, the `object.to_dict()` method must be used in place of `value(object)` for dictionary conversion.













### Optimizing Standard Benchmark Functions

Once OptiPy has been installed, it is ready to run optimizations. Firstly, you must import OptiPy and choose a problem to optimize. To view all available Optimization problems, you can execute the following, printing the Problem enum as a list:

```python
import opti_py

print(list(opti_py.Problem))
```

### Creating an Optimizer


Once you have selected a problem to optimize, you can create an ExperimentConfig object. ExperimentConfig objects take the parameters which are agnostic of which specific optimizer you use. This can be done as follows:

```python
config: opti_py.ExperimentConfig = opti_py.ExperimentConfig(
    problem_type=opti_py.Problem.SCHWEFEL,
    dimensions=30,
    lower=-512,
    upper=512,
    max_iterations=5000,
    seed=42
)
```

After creating an ExperimentConfig object, you are now able to instantiate an Optimizer object. Currently, OptiPy only has support for basic Particle Swarm Optimization and a few different methods of Differential Evolution, with support for Blind and Repeated Local Searches coming in the next release. A Particle Swarm optimizer can be created in the following manner:

```python
pso: opti_y.ParticleSwarm = ParticleSwarm(
    config,
    c1=0.8,
    c2=1.2,
    pop_size=200
)
```

Using default constructor values, an identical optimizer can be created with:

```python
pso: opti_py.ParticleSwarm = ParticleSwarm(
    config
)
```

The constructor for Differential Evolution optimizer varies slightly, as more methods are supported. The following code creates a Differential Evolution optimizer:

```python
de: opti_py.DifferentialEvolution = DifferentialEvolution(
    config,
    scale=0.9,
    crossover_rate=0.6,
    pop_size=200,
    mutation="rand1",
    crossover="bin"
)
```

Supported mutation methods are as follows:

- rand1
- rand2
- best1
- best2
- randToBest1

These can be passed as strings to the Differential Evolution constructor. Additionally, "bin" or "exp" can be passed as the crossover argument to allow for binomial or exponential crossover during optimization.


Similarly to ParticleSwarm objects, default constructor values can be used to create an identical object:


```python
de: opti_py.DifferentialEvolution = DifferentialEvolution(
    config
)
```

### Using an Optimizer

Optimizer objects all support the same methods, allowing polymorphic use through Python's duck-typing system. These methods are:

- `def optimize(self) -> NDArray[np.float64]`

    This allows users to run Optimization experiments. Taking no parameters, it returns an `ndarray` containing the best fitness value found at each optimization iteration.

- `def get_best_fitness(self) -> float`

    This method returns the most optimal fitness value found in the last optimization.

- `def get_fitnesses(self) -> list[float]`

    Returns the best fitness found at each iteration in Python list format. It will return the same values as the last optimize() call just in list format.

- `def get_best_solution(self) -> list[float]`

    Returns the most optimal solution found during the last optimization in list format. List length will match the number of dimensions used in the optimization.


Project Structure


```python
.
├── CMakeLists.txt # Build file for OptiPy. Controls how C++ code is compiles
├── README.md # Instructions for using OptiPy
├── example_usage # Example usage of OptiPy
│   └── flowshop # Example flow shop optimizations
│       ├── __init__.py # Empty file allowing directory to be treated as python package
│       ├── __main__.py # Main method for running example flow shop experiment
│       └── run_benchmarks.py # Runs flow shop experiment with input test data
|
├── pyproject.toml # pip library configuration
└── src # Contains all project source code
    └── opti_py
        ├── __init__.py # Defines which objects are exported by default for ease of use
        ├── cpp # Contains C++ code used in library
        │   ├── bindings.cpp # Python bindings for C++ code. Defines Python interface to library
        │   ├── include # Contains all heder files and C++ class declarations
        │   │   └── opti_py
        │   │       ├── ExperimentConfig.h # Experiment Configuration for function optimizations
        │   │       ├── External # Contains header files form external sources
        │   │       │   └── mt.h # Header file for external Mersenne Twister pseudo-random generator
        │   │       ├── FlowShop # Header files for flow shop related classes 
        │   │       │   ├── FlowShop.h # Class declaration for FlowShop
        │   │       │   └── FlowShopResult.h # Class declaration for FlowShopResult
        │   │       ├── Optimizer # Contains Optimizer functions
        │   │       │   ├── Blind.h # Class declaration for Blind Search optimizer
        │   │       │   ├── Crossover # Stores Crossover types for Differential Evolution
        │   │       │   │   ├── AllCrossovers.h # Allows for simple include of all crossover types
        │   │       │   │   ├── BinCrossover.h # Single-header file for binomial crossover
        │   │       │   │   ├── Crossover.h # Abstract base class for crossover methods
        │   │       │   │   └── ExpCrossover.h # Single-header file for exponential crossover
        │   │       │   ├── DifferentialEvolution.h # Class declaration for Differential Evolution optimizer
        │   │       │   ├── LocalSearch.h # Class declaration for Local Search optimizer
        │   │       │   ├── Mutation # Stores mutation types for Differential Evolution
        │   │       │   │   ├── AllMutations.h # Allows for simple include of all mutation types
        │   │       │   │   ├── Best1.h # Mutation implementation for best/1 
        │   │       │   │   ├── Best2.h # Mutation implementation for best/2
        │   │       │   │   ├── Mutation.h # Abstract base class for mutation methods
        │   │       │   │   ├── Rand1.h # Mutation implementation for rand/1
        │   │       │   │   ├── Rand2.h # Mutation implementation for rand/2
        │   │       │   │   └── RandBest1.h  # Mutation implementation for rand-to-best/1
        │   │       │   ├── Optimizer.h # Abstract base class for Optimizer types
        │   │       │   ├── OptimizerFactory.h # Constructs Optimizer objects of various subtypes
        │   │       │   └── ParticleSwarm.h # Class declaration for Local Search optimizer
        │   │       ├── Problem # Contains polymorphic standard benchmark function class definitions
        │   │       │   ├── AckleyOne.h # Implements Ackley's One standard benchmark function
        │   │       │   ├── AckleyTwo.h # Implements Ackley's Two standard benchmark function
        │   │       │   ├── DeJongOne.h # Implements De Jong One standard benchmark function
        │   │       │   ├── EggHolder.h # Implements Egg Holder standard benchmark function
        │   │       │   ├── Griewangk.h # Implements Griewangk standard benchmark function
        │   │       │   ├── Problem.h # Abstract base class for Problem classes
        │   │       │   ├── Rastrigin.h # Implements Rastrigin standard benchmark function
        │   │       │   ├── Rosenbrock.h # Implements Rosenbrock standard benchmark function
        │   │       │   ├── Schwefel.h # Implements Schwefel standard benchmark function
        │   │       │   ├── SineEnvelope.h # Implements Sine Envelope Sine Wave standard benchmark function
        │   │       │   └── StretchedV.h # Implements Stretched V Sine Wave standard benchmark function
        │   │       ├── ProblemFactory.h # Constructs various Problem class implementations form problem ID
        │   │       ├── SolutionBuilder.h # Functionality for generating and modifying high-dimensional solutions
        │   │       └── debug.h # Used for debug logging when env var set
        │   └── src # Contains C++ implementation files used by OptiPy library
        │       ├── External # Contains external source code
        │       │   └── mt.cpp # Implementation file for external Mersenne Twister pseudo-random generator
        │       ├── FlowShop # Implementation files for flow shop problem
        │       │   └── FlowShop.cpp # C++ implementation for FlowShop class
        │       ├── Optimizer # Contains implementation files for Optimizer classes
        │       │   ├── Blind.cpp # C++ implementation for Blind Search optimizer
        │       │   ├── DifferentialEvolution.cpp # C++ implementation for Differential Evolution optimizer 
        │       │   ├── LocalSearch.cpp # C++ implementation for Local Search optimizer
        │       │   └── ParticleSwarm.cpp # C++ implementation for Particle Swarm optimizer
        │       ├── ProblemFactory.cpp # C++ source code for ProblemFactory class
        │       └── SolutionBuilder.cpp # C++ source code for SolutionBuilder class
        └── problem.py # Python enum for available standard benchmark function problem types

18 directories, 52 files

```



# Flow Shop Example Experiment

A command-line tool for running and analyzing flow shop scheduling optimization experiments with differential evolution algorithms. Supports parameter grid exploration, multi-seed averaging, algorithm comparison, and result visualization.

## Overview

This program orchestrates a complete benchmarking pipeline:

1. **Load Configuration** — Read TOML-based experiment parameters and algorithm settings
2. **Run Experiments** — Execute DE-based optimization across multiple problem instances and seeds
3. **Aggregate Results** — Average metrics across seeds and compare algorithm variants
4. **Generate Analysis** — Produce plots, statistics, and comparison reports

The pipeline is designed for flexibility: run experiments in isolation, reload and re-analyze previous results, or skip to analysis if you already have data.

## Installation & Setup

Before running the experiment, required packages must be installed. It is highly recommended some form of isolated environment is used to install dependencies in order to avoid bloating your global python installation. A `venv` can be created and activated with:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

 The `example_usage` directory contains a `requirements.tx` file which can be used in the following manner to install dependencies:

```bash
cd example_usage
pip install -r requirements.txt
```

However, because the OptiPy library is not yet publicly available, it must be installed separately. Navigating to the root of the repository, this can be done with:

```bash
pip install .
```


## Usage

```bash
python -m your_module [OPTIONS]
```

## Arguments

### Configuration & Data Paths

| Flag | Full Name | Type | Default | Description |
|------|-----------|------|---------|-------------|
| `-c` | `--config` | Path | `./config.toml` | TOML configuration file defining parameter grids, DE settings, and experiment metadata |
| `-d` | `--data` | Path | `./data` | Directory containing flow shop problem instances (.txt files) |
| `-o` | `--output` | Path | `./results` | Output directory where results, plots, and analysis are saved |

### Execution Modes

| Flag | Full Name | Type | Description |
|------|-----------|------|-------------|
| `-e` | `--experiment-only` | Flag | Run experiments and save results; **skip** averaging and analysis phases. |
| `-l` | `--load-results` | Path | Load pre-computed results from a pickle file (skips experiment execution entirely). Useful for re-analyzing or changing visualization parameters without re-running benchmarks. |

### Debugging & Output

| Flag | Full Name | Type | Description |
|------|-----------|------|-------------|
| `-v` | `--verbose` | Flag | Print detailed progress messages during experiment execution (one line per benchmark). Helps monitor long-running jobs. |
| `-dbg` | `--debug` | Flag | Print full DataFrame snapshots at three stages: raw results, averaged results, and algorithm comparison. Use to inspect data structure and content between pipeline phases. |

## Examples

### Example 1: Full Pipeline (Default)

Run experiments, analyze, and generate reports in one go:

```bash
python -m your_module -c config.toml -d ./data -o ./results
```

**What happens:**
- Loads `config.toml` and discovers all parameter combinations
- Iterates over flow shop problems in `./data`
- Executes DE for each parameter set and seed
- Saves raw results to `./results/results.pkl`
- Averages across seeds
- Generates algorithm comparison and plots
- Writes reports to `./results`

---

### Example 2: Experiments Only

Run experiments without analysis, results can be loaded and analyzed later

```bash
python -m your_module -c config.toml -d ./data -o ./results -e
```

**What happens:**
- Executes all benchmarks
- Saves `./results/results.pkl` with raw data
- **Stops** — no analysis or plotting
- Exit code: 0

**Use case:** Run multiple instances in parallel, then combine results and analyze separately.

---

### Example 3: Load & Re-Analyze

Skip experiments; load previous results and regenerate analysis:

```bash
python -m your_module -l ./results/results.pkl -o ./new_analysis
```

**What happens:**
- Ignores config and data directory
- Loads raw results from pickle
- Re-averages and re-generates plots
- Saves to `./new_analysis`

**Use case:** Change plotting style, adjust threshold parameters, or fix analysis without re-running expensive experiments.

---

### Example 4: Verbose + Debug

Full introspection into data at each stage:

```bash
python -m your_module -c config.toml -d ./data -o ./results -v -dbg
```

**What happens:**
- Prints one line per benchmark executed (verbose)
- After experiments complete, prints raw results DataFrame (shape, dtypes, head/tail)
- After averaging, prints averaged DataFrame
- After aggregation, prints algorithm comparison DataFrame
- Displays exact plots being created
- Useful for verifying data integrity and detecting missing/malformed entries

---

### Example 5: Custom Paths

Use non-standard directory layout:

```bash
python -m your_module \
  -c /path/to/experiments/tuning.toml \
  -d /mnt/benchmarks/flowshop_instances \
  -o /mnt/results/run_2024
```

---

## Configuration File (config.toml)

The TOML configuration specifies parameter grids and experiment metadata:

```toml
[param_grid]
blocking = [true, false]
tardiness = [true, false]
max_gens = [500]
pop_size = [200]
f = [0.8]
cr = [0.5]
seed = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
mutation = ["rand1", "best1", "rand2", "best2", "randToBest1"]
crossover = ["bin", "exp"] 


```

The program generates a Cartesian product of all parameter combinations and executes one benchmark per (instance, param_set, seed) tuple.



## Error Handling

The program exits gracefully with informative messages:

| Condition | Exit Code | Message |
|-----------|-----------|---------|
| Config file not found | 1 | `Config file not found: [path]` |
| Invalid config TOML | 1 | `Invalid configuration: [error]` |
| Data directory missing | 1 | `Invalid Argument: Path must point to directory...` |
| Pickle file corrupted | 1 | `Failed to unpickle file (corrupted?): [error]` |
| Unexpected exception | 1 | `Unexpected error occurred: [error]` |


## Troubleshooting

### "Pickle file not found"

Verify the pickle path exists and is readable:
```bash
ls -lh ./results/results.pkl
```

### Plots are missing

Check that `build_results()` succeeded (look for errors in console output). Ensure matplotlib and seaborn are installed:
```bash
pip install matplotlib seaborn
```

### Memory usage grows unbounded

If processing very large result sets, consider splitting analysis:
1. Run experiments in batches with `-e`
2. Analyze each batch separately with `-l`

---

## Dependencies

- `pandas` — Data manipulation and aggregation
- `argparse` — Command-line argument parsing
- `pathlib` — Path handling
- `matplotlib` & `seaborn` — Plotting (for `build_results()`)
- `numpy` — Numerical operations (often via pandas/matplotlib)

Install with:
```bash
pip install pandas matplotlib seaborn numpy
```

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Error (config, data, or runtime) |

---

## See Also

- **Configuration Guide:** See `config.toml` comments for parameter grid syntax
- **Algorithm Documentation:** Refer to `run_benchmarks.py` for DE variant details
- **Data Format:** Problem instances should be plain text (format specified in `run_benchmarks.py`)
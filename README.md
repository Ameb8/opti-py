# OptiPy

OptiPy is a Python Optimization library designed to handle both the speed and efficiency of C++ code with the usability of Python code. It has support for minimizing standard benchmark functions, as well as optimizing the flow shop problem. The public interface provided by OptiPy is entirely in Python, including support for creating and running Optimizer objects.

## Using OptiPy

### Installation

As this package is not yet available on `pip`, source code must be downloaded in order to install it. The full source code for this library can be found [on the project's Github page](https://github.com/Ameb8/opti-py). After downloading the source code, it can be installed with pip in the following manner:

```bash
pip install <path to cloned repository root>
```

after installed with `pip`, it can be used in any directory location as long as you are using the same Python environment in which it was installed. All C++ code is compiled automatically during the installation process.

### Requirements

While older versions may work, this program officially supports only Python 3.11 and newer. Additionally, `pip` must be  installed and up to date. The package only officially supports MacOS and Linux, but may work on Windows systems.


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

A Default constructor taking no arguments can optionally be invoked, however, a numpy array must then be assigned to the jobs field before flow shop calcultions can be made. Thus, the following code creates a FlowShop object identical to the one above.

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


#### Optimizing the Flow Shop Problem

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

The same FlowShop object can be used to conduct any number of optimizations, with or without modifications to the `jobs` field matrix in between runs. Modifications to a FlowShop object will not affect FlowShopResult objects that it has already created. 

#### Flow Shop Results

| Field | Type | Description |
| :--- | :--- | :--- |
| **sequence** | *list[int]* | Order in which jobs are executed. *sequence[i]* represents the index of the job iin the original *FlowShop.jobs* table | 
| **makespan** | *int* | Total amount of time needed to execute all jobs on all machines |
| **completion_times** | *list[list[int]]* | The time from start each job finishes on each machine. *completion_times[i][j]* is the time job *i* finishes execution on machine *j*. Job *i* refers to the job scheduled in the *ith* position, not the *ith* row in original jobs matrix.|

In addition to data fields, the FlowShopResult class offers a `to_dict()` method. When invoked, this method constructs a python dictionary of type *dict[str, Any]*, with Keys exactly matching field names and values matching the field values. As the class has no underlying `__dict__` property like native Python classes, the `object.to_dict()` method must be used in place of `value(object)` for dictionary conversion.


### Optimizing Standard Benchmark Functions

Once OptiPy has been installed, it is ready to run optimizations. Firstly, you must import OptiPy and choose a problem to optimize. To view all available Optimization problems, you can execute the following, printing the Problem enum as a list:

```python
import opti_py

print(list(opti_py.Problem))
```

### Creating an Optimizer


Once you have selected a problem to optimize, you can create an ExperimentConfig object. ExperimentConfig objects take the parameters whiich are agnostic of which specific optimizer you use. This can be done as follows:

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


Similarly to ParticleSwarm objects, default constructor values cana be used to create an identical object:


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



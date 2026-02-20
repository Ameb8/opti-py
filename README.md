# OptiPy

OptiPy is a Python library designed to handle both the speed and efficiency of C++ code with the usability of Python code. The public interface provided by OptiPy is entirely in Python, including support for creating and running Optimizer objects.

## Using OptiPy

### Installation

As this package is not yet available on `pip`, source code must be downloaded in order to install it. The full source code for this library can be found [on the project's Github page](https://github.com/Ameb8/opti-py). After downloading the source code, it can be installed with pip n the following manner:

```bash
pip install <path to cloned repository root>
```

after installed with `pip`, it can be used in any directory location as long as you are using the same Python environment in which it was installed. All C++ code is compiled automatically during the installation process.

### Requirements

While older versions may work, this program officially supports only Python 3.11 and newer. Additionally, `pip` must be  installed and up to date. The package only officially supports MacOS and Linux, but may work on Windows systems.


## Running Optimizations

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
pso: opti_y.ParticleSwarm = ParticleSwarm(
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


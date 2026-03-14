# Expose Optimizer algorithms
from ._opti_py import (
    ParticleSwarm,
    ExperimentConfig,
    FlowShop,
    FlowShopResult,
    optimize_de,
    OptResult
)

# Expose problem ID mapping Enum
from .problem import Problem

# Expose DE strategies mapping Enum
from .de_strats import Mutation, Crossover

# Import and attach FlowShop extension methods
from .flowshop_ext import set_due_dates
FlowShop.set_due_dates = set_due_dates
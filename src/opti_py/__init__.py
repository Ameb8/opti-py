# Expose Optimizer algorithms
from ._opti_py import (
    ParticleSwarm,
    ExperimentConfig,
    FlowShop,
    FlowShopResult,
    optimize_de,
    ProblemResult
)

# Expose problem ID mapping Enum
from .problem import Problem
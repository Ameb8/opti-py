from opti_py import Mutation, Crossover

from dataclasses import dataclass, asdict
from enum import Enum


@dataclass
class Experiment:
    blocking: bool
    optimize_tardiness: bool
    max_gens: int
    pop_size: int
    f: float
    cr: float
    seed: int
    mutation: Mutation
    crossover: Crossover

    def to_dict(self) -> dict:
        result = asdict(self)
        for key, val in result.items():
            if isinstance(val, Enum):
                result[key] = val.value
        return result


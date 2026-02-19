from enum import IntEnum


class Problem(IntEnum):
    SCHWEFEL = 1
    DEJONG_ONE = 2
    ROSENBROCK = 3
    RASTRIGIN = 4
    GRIEWANGK = 5
    SINE_ENVELOPE = 6
    STRETCHED_V = 7
    ACKLEY_ONE = 8
    ACKLEY_TWO = 9
    EGGHOLDER = 10

    def __str__(self):
        # Make it lowercase except first letter
        s = self.name.replace("_", " ").lower()
        return s.capitalize()
    
    @classmethod
    def from_string(cls, name: str):
        normalized = name.replace(" ", "_").upper()
        return cls[normalized]

from enum import Enum

class Mutation(str, Enum):
    RAND1 = 'rand1'
    RAND2 = 'rand2'
    BEST1 = 'best1'
    BEST2 = 'best2'
    RAND_TO_BEST1 = 'randToBest1'

class Crossover(str, Enum):
    BIN = 'bin'
    EXP = 'exp'
"""Load parameter grid configuration from TOML file with type validation."""
from opti_py import Mutation, Crossover

from enum import Enum
from pathlib import Path
from typing import Any


class ParamGridError(Exception):
    """Raised when parameter grid values have incorrect types."""

    pass


def load_param_grid(config_path: Path) -> dict[str, Any]:
    try:
        import tomllib
    except ImportError:
        import tomli as tomllib  # type: ignore

    config_path = Path(config_path)

    if not config_path.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_path}")

    with open(config_path, "rb") as f:
        config = tomllib.load(f)

    if "param_grid" not in config:
        raise ParamGridError("TOML file must contain [param_grid] section")

    param_grid = config["param_grid"]

    # Type validators for each parameter
    validators = {
        "blocking": (list, bool),
        "tardiness": (list, bool),
        "max_gens": (list, int),
        "pop_size": (list, int),
        "seed": (list, int),
        "f": (list, (int, float)),
        "cr": (list, (int, float)),
        "mutation": (list, str, Mutation),
        "crossover": (list, str, Crossover),
    }

    # Validate and convert each parameter
    for param_name, validator_spec in validators.items():
        if param_name not in param_grid:
            continue

        value = param_grid[param_name]

        if not isinstance(value, list):
            raise ParamGridError(f"'{param_name}' must be a list")

        if len(validator_spec) == 2:
            # Simple type: (list, element_type)
            _, elem_type = validator_spec
            
            # Special handling for booleans (don't exclude them)
            if elem_type is bool:
                if not all(isinstance(v, bool) for v in value):
                    raise ParamGridError(f"'{param_name}' must contain bool values")
            else:
                # For numeric types, exclude booleans (True/False aren't valid 0/1)
                if not all(isinstance(v, elem_type) and not isinstance(v, bool) for v in value):
                    raise ParamGridError(f"'{param_name}' must contain {elem_type.__name__} values")

                # Coerce ints to floats for f and cr
                if param_name in ("f", "cr"):
                    param_grid[param_name] = [float(v) for v in value]


        elif len(validator_spec) == 3:
            # Enum type: (list, string, EnumClass)
            _, str_type, enum_class = validator_spec
            if not all(isinstance(v, str_type) for v in value):
                raise ParamGridError(f"'{param_name}' must contain strings")
            try:
                param_grid[param_name] = [enum_class(v) for v in value]
            except ValueError as e:
                valid = [e.value for e in enum_class]
                raise ParamGridError(
                    f"Invalid {param_name} value. Valid: {valid}"
                ) from e

    return param_grid
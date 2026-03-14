import numpy as np


def set_due_dates(self, rho: float = 0.5, seed: int = 108664) -> None:
    # Create seeded rng
    rng = np.random.default_rng(seed)

    # Sum processing times for each job
    total_times = self.jobs.sum(axis=1)

    # Generate random values for each job
    random_vals = rng.random(self.num_jobs)

    # Compute due dates
    self.due_dates = np.floor(
        rho * total_times * (1 + random_vals)
    ).astype(self.jobs.dtype)
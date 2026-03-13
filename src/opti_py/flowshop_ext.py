import numpy as np


def set_due_dates(self, rho: float = 2.5, seed: int = 108664) -> None:
    # Create seeded rng
    rng = np.random.default_rng(seed)

    # Create uninitialized due_dates matrix
    self.due_dates = np.empty(self.num_jobs, dtype=self.jobs.dtype)

    # iterate through jobs
    for i in range(self.num_jobs):
        total_time = 0
        # Calculate total processing time of job
        for j in range(self.num_machines):
            total_time += self.jobs[i][j]

        # Generate due date
        due_date: float = rho * total_time * (1 + rng.random())
        self.due_dates[i] = int(np.floor(due_date))


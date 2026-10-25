# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM compatibility constructor for MQT Core's exact V2 estimator."""

from mqt.core.plugins.qiskit import DDEstimator


class Estimator(DDEstimator):
    """Return exact expectation values with zero sampling uncertainty.

    Positive precision is a requested error bound; results remain exact. Use the
    qasm backend's estimator() factory for sampled expectation values.
    """

    def __init__(self, *, default_precision: float = 0.0, seed: int | None = -1) -> None:
        """Create an exact estimator; the compatibility seed has no effect."""
        super().__init__(default_precision=default_precision)
        self._seed = seed

    @property
    def seed(self) -> int | None:
        """Compatibility seed, retained unchanged and unused by exact execution."""
        return self._seed

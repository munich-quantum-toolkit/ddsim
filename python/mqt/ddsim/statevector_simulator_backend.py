# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM uncollapsed statevector backend."""

from ._dense_backend import _DenseSimulatorBackend


class StatevectorSimulatorBackend(_DenseSimulatorBackend):
    """Return the exact statevector, retaining source global phase and qubit width."""

    def __init__(self) -> None:
        """Create a statevector backend."""
        super().__init__()

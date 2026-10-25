# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM full unitary backend."""

from ._dense_backend import _DenseSimulatorBackend


class UnitarySimulatorBackend(_DenseSimulatorBackend):
    """Return a full unitary; measurements, resets, and sampling options are invalid."""

    def __init__(self) -> None:
        """Create a unitary backend."""
        super().__init__(unitary=True)

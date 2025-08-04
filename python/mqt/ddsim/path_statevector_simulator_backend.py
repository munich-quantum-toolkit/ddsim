# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Qiskit backend for the MQT DDSIM simulation-path statevector simulator."""

from __future__ import annotations

from qiskit.transpiler import Target

from .path_qasm_simulator_backend import PathQasmSimulatorBackend


class PathStatevectorSimulatorBackend(PathQasmSimulatorBackend):
    """Qiskit backend for the MQT DDSIM simulation-path statevector simulator."""

    _SHOW_STATE_VECTOR = True
    _Path_SV_TARGET = Target(
        description="Target for the MQT DDSIM simulation-path statevector simulator",
        num_qubits=30,  # corresponds to 16GiB memory for storing the full statevector
    )

    def __init__(self) -> None:
        """Constructor for the MQT DDSIM simulation-path statevector simulator."""
        super().__init__(
            name="path_statevector_simulator",
            description="MQT DDSIM simulation-path statevector simulator",
        )

    @property
    def target(self) -> Target:
        """Return the target of the backend."""
        return self._Path_SV_TARGET

# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Qiskit backend for MQT DDSIM statevector simulators."""

from __future__ import annotations

from qiskit.transpiler import Target

from .qasm_simulator_backend import QasmSimulatorBackend


class StatevectorSimulatorBackend(QasmSimulatorBackend):
    """Qiskit backend for MQT DDSIM statevector simulators."""

    _SHOW_STATE_VECTOR = True
    _SV_TARGET = Target(
        description="Target for the MQT DDSIM statevector simulator",
        num_qubits=30,  # corresponds to 16GiB memory for storing the full statevector
    )

    def __init__(
        self,
        name: str = "statevector_simulator",
        description: str = "MQT DDSIM statevector simulator",
    ) -> None:
        """Constructor for the MQT DDSIM statevector simulator backend."""
        super().__init__(name=name, description=description)

    @property
    def target(self) -> Target:
        """Return the target of the backend."""
        return self._SV_TARGET

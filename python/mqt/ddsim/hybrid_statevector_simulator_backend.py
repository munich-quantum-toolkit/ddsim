# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Qiskit backend for the MQT DDSIM hybrid Schrodinger-Feynman statevector simulator."""

from __future__ import annotations

from qiskit.transpiler import Target

from .hybrid_qam_simulator_backend import HybridQasmSimulatorBackend


class HybridStatevectorSimulatorBackend(HybridQasmSimulatorBackend):
    """Qiskit backend for the MQT DDSIM hybrid Schrodinger-Feynman statevector simulator."""

    _SHOW_STATE_VECTOR = True
    _HSF_SV_TARGET = Target(
        description="Target for the MQT DDSIM hybrid Schrodinger-Feynman statevector simulator",
        num_qubits=30,  # corresponds to 16GiB memory for storing the full statevector
    )

    def __init__(
        self,
        name: str = "hybrid_statevector_simulator",
        description: str = "MQT DDSIM hybrid Schrodinger-Feynman statevector simulator",
    ) -> None:
        """Constructor for the MQT DDSIM hybrid Schrodinger-Feynman statevector simulator backend."""
        super().__init__(
            name=name,
            description=description,
        )

    @property
    def target(self) -> Target:
        """Return the target of the backend."""
        return self._HSF_SV_TARGET

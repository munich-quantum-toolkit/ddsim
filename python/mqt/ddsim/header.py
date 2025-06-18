# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Utilities for constructing a Qiskit experiment header for DDSIM backends."""

from __future__ import annotations

from dataclasses import asdict, dataclass
from typing import TYPE_CHECKING, Any

try:
    from qiskit.result.models import QobjExperimentHeader

    QISKIT_PRE_2_0 = True
except ImportError:
    QobjExperimentHeader = object
    QISKIT_PRE_2_0 = False

if TYPE_CHECKING:
    from qiskit import QuantumCircuit


@dataclass
class DDSIMHeader(QobjExperimentHeader):  # type: ignore[misc]
    """Header for DDSIM backends."""

    name: str
    n_qubits: int
    memory_slots: int
    global_phase: float
    creg_sizes: list[tuple[str, int]]
    clbit_labels: list[tuple[str, int]]
    qreg_sizes: list[tuple[str, int]]
    qubit_labels: list[tuple[str, int]]

    @classmethod
    def from_quantum_circuit(cls, quantum_circuit: QuantumCircuit) -> DDSIMHeader:
        """Create a DDSIM experiment header from a QuantumCircuit."""
        return cls(
            name=quantum_circuit.name,
            n_qubits=quantum_circuit.num_qubits,
            memory_slots=quantum_circuit.num_clbits,
            global_phase=quantum_circuit.global_phase,
            creg_sizes=[(creg.name, creg.size) for creg in quantum_circuit.cregs],
            clbit_labels=[(creg.name, j) for creg in quantum_circuit.cregs for j in range(creg.size)],
            qreg_sizes=[(qreg.name, qreg.size) for qreg in quantum_circuit.qregs],
            qubit_labels=[(qreg.name, j) for qreg in quantum_circuit.qregs for j in range(qreg.size)],
        )

    def get_compatible_version(self) -> QobjExperimentHeader | dict[str, Any]:
        """Return a compatible header.

        For Qiskit < 2.0, return a QobjExperimentHeader. For Qiskit >= 2.0, return a dict.
        """
        if QISKIT_PRE_2_0:
            return self

        return asdict(self)

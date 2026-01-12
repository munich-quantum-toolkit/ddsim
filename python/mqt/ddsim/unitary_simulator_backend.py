# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Qiskit backend for the MQT DDSIM unitary simulator."""

from __future__ import annotations

import time
from typing import TYPE_CHECKING, Any

import numpy as np
from mqt.core import load
from qiskit import QiskitError
from qiskit.providers import Options
from qiskit.result.models import ExperimentResult, ExperimentResultData
from qiskit.transpiler import Target

from .experiment_header import DDSIMExperimentHeader
from .pyddsim import UnitarySimulator, UnitarySimulatorMode
from .qasm_simulator_backend import QasmSimulatorBackend
from .target import DDSIMTargetBuilder

if TYPE_CHECKING:
    from collections.abc import Sequence

    from qiskit import QuantumCircuit


class UnitarySimulatorBackend(QasmSimulatorBackend):
    """Qiskit backend for the MQT DDSIM unitary simulator."""

    _US_TARGET = Target(
        description="Target for the MQT DDSIM unitary simulator",
        num_qubits=15,  # corresponds to 16GiB memory for storing the full matrix
    )

    @staticmethod
    def _add_operations_to_target(target: Target) -> None:
        DDSIMTargetBuilder.add_0q_gates(target)
        DDSIMTargetBuilder.add_1q_gates(target)
        DDSIMTargetBuilder.add_2q_gates(target)
        DDSIMTargetBuilder.add_3q_gates(target)
        DDSIMTargetBuilder.add_multi_qubit_gates(target)
        DDSIMTargetBuilder.add_barrier(target)

    def __init__(
        self,
        name: str = "unitary_simulator",
        description: str = "MQT DDSIM unitary simulator",
    ) -> None:
        """Constructor for the MQT DDSIM unitary simulator backend."""
        super().__init__(name=name, description=description)

    @classmethod
    def _default_options(cls) -> Options:
        return Options(shots=1, mode="recursive", parameter_binds=None)

    @property
    def target(self) -> Target:
        """Return the target of the backend."""
        return self._US_TARGET

    @classmethod
    def _run_experiment(cls, qc: QuantumCircuit, **options: Any) -> ExperimentResult:
        start_time = time.time()
        seed = options.get("seed", -1)
        mode = options.get("mode", "recursive")

        if mode == "sequential":
            construction_mode = UnitarySimulatorMode.sequential
        elif mode == "recursive":
            construction_mode = UnitarySimulatorMode.recursive
        else:
            msg = (
                f"Construction mode {mode} not supported by DDSIM unitary simulator. Available modes are "
                "'recursive' and 'sequential'"
            )
            raise QiskitError(msg)

        circuit = load(qc)
        sim = UnitarySimulator(circuit, seed=seed, mode=construction_mode)
        sim.construct()
        # Extract resulting matrix from final DD and write data
        dd = sim.get_constructed_dd()
        mat = dd.get_matrix(sim.get_number_of_qubits())
        unitary = np.array(mat, copy=False)
        end_time = time.time()

        data = ExperimentResultData(
            unitary=unitary,
            construction_time=sim.get_construction_time(),
            dd_nodes=sim.get_final_node_count(),
            time_taken=end_time - start_time,
        )

        return ExperimentResult(
            shots=1,
            success=True,
            status="DONE",
            seed=seed,
            data=data,
            metadata=qc.metadata,
            header=DDSIMExperimentHeader.from_quantum_circuit(qc).get_compatible_version(),
        )

    def _validate(self, quantum_circuits: Sequence[QuantumCircuit]) -> None:
        """Semantic validations of the quantum circuits which cannot be done via schemas.

        1. No shots
        2. No measurements in the middle.
        """
        for qc in quantum_circuits:
            name = qc.name
            n_qubits = qc.num_qubits
            max_qubits = self.target.num_qubits

            if n_qubits > max_qubits:
                msg = f"Number of qubits {n_qubits} is greater than maximum ({max_qubits}) for '{self.name}'."
                raise QiskitError(msg)

            if qc.metadata is not None and "shots" in qc.metadata and qc.metadata["shots"] != 1:
                qc.metadata["shots"] = 1

            for instruction in qc.data:
                operation = instruction.operation
                if operation.name in {"measure", "reset"}:
                    msg = f"Unsupported '{self.name}' instruction '{operation.name}' in circuit '{name}'."
                    raise QiskitError(msg)

# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Qiskit result adaptation for MQT Core's exact dense simulation functions."""

from __future__ import annotations

from importlib.metadata import version
from typing import TYPE_CHECKING
from uuid import uuid4

from qiskit import QuantumCircuit
from qiskit.providers import BackendV2, Options
from qiskit.providers.basic_provider import BasicProviderJob
from qiskit.result import Result
from qiskit.result.models import ExperimentResult, ExperimentResultData
from qiskit.transpiler import Target

from .qasm_simulator_backend import QasmSimulatorBackend
from .simulation import build_functionality, simulate

if TYPE_CHECKING:
    from collections.abc import Mapping, Sequence

    from qiskit.circuit import Parameter


class _DenseSimulatorBackend(BackendV2):
    """Return an exact dense state or unitary without a sampling pass."""

    def __init__(self, *, unitary: bool = False) -> None:
        """Create a dense backend using the Core device gate inventory."""
        self._unitary = unitary
        mode = "unitary" if unitary else "statevector"
        super().__init__(name=f"{mode}_simulator", backend_version=version("mqt-ddsim"))
        self._target = Target(description=f"MQT DDSIM {mode} simulator", num_qubits=15 if unitary else 30)
        source = QasmSimulatorBackend().target
        for name in source.operation_names:
            if name != "reset" and (not unitary or name != "measure"):
                self._target.add_instruction(source.operation_from_name(name), name=name)

    @classmethod
    def _default_options(cls) -> Options:
        return Options()

    @property
    def target(self) -> Target:
        """The target supported by the simulation engine."""
        return self._target

    @property
    def max_circuits(self) -> None:
        """No fixed limit on the number of circuits in a job."""
        return None

    def run(
        self,
        run_input: QuantumCircuit | Sequence[QuantumCircuit],
        parameter_values: Sequence[Sequence[float] | Mapping[Parameter, float] | None] | None = None,
        **options: object,
    ) -> BasicProviderJob:
        """Return dense results synchronously, without shots or count data.

        Args:
            run_input: One circuit or a nonempty batch.
            parameter_values: One binding per circuit, in circuit.parameters order.
            options: No execution options are supported; sample separately for counts.
        """
        if options:
            msg = f"Dense simulation does not accept {', '.join(sorted(options))}; use sample or qasm_simulator for counts"
            raise ValueError(msg)
        circuits = [run_input] if isinstance(run_input, QuantumCircuit) else list(run_input)
        if not circuits or any(not isinstance(circuit, QuantumCircuit) for circuit in circuits):
            msg = "run_input must contain at least one QuantumCircuit"
            raise ValueError(msg)
        if parameter_values is not None:
            if len(parameter_values) != len(circuits):
                msg = "parameter_values must have one entry per circuit"
                raise ValueError(msg)
            bound_circuits = []
            for circuit, values in zip(circuits, parameter_values, strict=True):
                bound = circuit.assign_parameters(values) if values is not None else circuit
                bound.name = circuit.name
                bound_circuits.append(bound)
            circuits = bound_circuits
        for circuit in circuits:
            if circuit.num_parameters:
                msg = "Circuit parameters must be bound before simulation"
                raise ValueError(msg)
            if circuit.num_qubits > self.num_qubits:
                msg = f"Dense {self.name} supports at most {self.num_qubits} qubits"
                raise ValueError(msg)
        results = []
        for circuit in circuits:
            values = build_functionality(circuit) if self._unitary else simulate(circuit)
            data = ExperimentResultData(**{"unitary" if self._unitary else "statevector": values})
            results.append(
                ExperimentResult(
                    shots=0,
                    success=True,
                    data=data,
                    header={
                        "name": circuit.name,
                        "n_qubits": circuit.num_qubits,
                        "memory_slots": circuit.num_clbits,
                        "creg_sizes": [[reg.name, len(reg)] for reg in circuit.cregs],
                        "metadata": circuit.metadata,
                    },
                )
            )
        job_id = str(uuid4())
        result = Result(
            backend_name=self.name,
            backend_version=self.backend_version,
            qobj_id=None,
            job_id=job_id,
            success=True,
            results=results,
        )
        return BasicProviderJob(self, job_id, result)

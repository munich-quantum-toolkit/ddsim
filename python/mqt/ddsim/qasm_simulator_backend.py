# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM sampling backend using MQT Core's QDMI device."""

from __future__ import annotations

from importlib.metadata import version
from numbers import Integral
from typing import TYPE_CHECKING

from qiskit.providers import Options

from mqt.core.plugins.qiskit import QDMIBackend
from mqt.core.qdmi import open_device

from .simulation import _seed, _shots

if TYPE_CHECKING:
    from collections.abc import Iterable, Mapping, Sequence

    from qiskit import QuantumCircuit
    from qiskit.circuit import Parameter
    from qiskit.circuit.parameterexpression import ParameterValueType

    from mqt.core.plugins.qiskit import QDMIJob
    from mqt.core.typing import QDMIJobParameters


class QasmSimulatorBackend(QDMIBackend):
    """Sample circuits on the MQT Core DD simulator.

    Circuits without classical bits are measured on a copy for legacy backend.run
    compatibility. Native sampler PUBs retain their declared classical registers.
    seed_simulator accepts None, -1, or a positive signed 32-bit integer.
    max_workers=0 selects automatic concurrency; use 1 for serial sampling.
    """

    def __init__(self) -> None:
        """Create a sampling backend with its own QDMI session."""
        super().__init__(open_device("mqt.ddsim.default"), device_id="mqt.ddsim.default")
        self.name = "qasm_simulator"
        self.description = "MQT DDSIM sampling simulator"
        self.backend_version = version("mqt-ddsim")

    @classmethod
    def _default_options(cls) -> Options:
        return Options(shots=1024, memory=False, max_retries=0, seed_simulator=None, max_workers=0)

    @staticmethod
    def _job_parameters(options: Mapping[str, object]) -> QDMIJobParameters:
        seed = _seed(options["seed_simulator"])
        workers = options["max_workers"]
        if isinstance(workers, bool) or not isinstance(workers, Integral) or int(workers) not in range(2**31):
            msg = "max_workers must be an integer from 0 to 2147483647 (0 selects automatic concurrency)"
            raise ValueError(msg)
        return {"custom1": seed, "custom3": int(workers)}

    @staticmethod
    def _preprocess_circuit(circuit: QuantumCircuit) -> QuantumCircuit:
        if not circuit.num_clbits:
            circuit = circuit.copy()
            circuit.measure_all()
        return circuit

    def run(
        self,
        run_input: QuantumCircuit | Sequence[QuantumCircuit],
        parameter_values: Sequence[Mapping[Parameter, ParameterValueType] | Iterable[ParameterValueType]] | None = None,
        **options: object,
    ) -> QDMIJob:
        """Sample bound circuits; parameters follow circuit.parameters order."""
        _shots(options.get("shots", self.options.shots))
        return super().run(run_input, parameter_values=parameter_values, **options)

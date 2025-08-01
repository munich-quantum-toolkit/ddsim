# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Backend for DDSIM Stochastic Simulator."""

from __future__ import annotations

import time
from typing import TYPE_CHECKING, Any, cast

from mqt.core import load
from qiskit.providers import Options
from qiskit.result.models import ExperimentResult, ExperimentResultData

from mqt.ddsim.pyddsim import StochasticNoiseSimulator

from .header import DDSIMHeader
from .qasmsimulator import QasmSimulatorBackend

if TYPE_CHECKING:
    from qiskit import QuantumCircuit


class StochasticNoiseSimulatorBackend(QasmSimulatorBackend):
    """Python interface to MQT DDSIM stochastic noise-aware simulator."""

    def __init__(
        self,
        name: str = "stochastic_dd_simulator",
        description: str = "MQT DDSIM noise-aware stochastic simulator based on decision diagrams",
    ) -> None:
        """Constructor for the DDSIM stochastic simulator backend."""
        super().__init__(name=name, description=description)

    @classmethod
    def _default_options(cls) -> Options:
        return Options(
            shots=None,
            parameter_binds=None,
            simulator_seed=None,
            approximation_step_fidelity=1.0,
            approximation_steps=1,
            approximation_strategy="fidelity",
            noise_effects="APD",
            noise_probability=0.01,
            amp_damping_probability=0.02,
            multi_qubit_gate_factor=2,
        )

    @staticmethod
    def _run_experiment(qc: QuantumCircuit, **options: dict[str, Any]) -> ExperimentResult:
        start_time = time.time()
        approximation_step_fidelity = cast("float", options.get("approximation_step_fidelity", 1.0))
        approximation_steps = cast("int", options.get("approximation_steps", 1))
        approximation_strategy = cast("str", options.get("approximation_strategy", "fidelity"))
        noise_effects = cast("str", options.get("noise_effects", "APD"))
        noise_probability = cast("float", options.get("noise_probability", 0.01))
        amp_damping_probability = cast("float", options.get("amp_damping_probability", 0.02))
        multi_qubit_gate_factor = cast("float", options.get("multi_qubit_gate_factor", 2))
        seed = cast("int", options.get("seed_simulator", -1))
        shots = cast("int", options.get("shots", 1024))

        circ = load(qc)
        sim = StochasticNoiseSimulator(
            circ=circ,
            approximation_step_fidelity=approximation_step_fidelity,
            approximation_steps=approximation_steps,
            approximation_strategy=approximation_strategy,
            seed=seed,
            noise_effects=noise_effects,
            noise_probability=noise_probability,
            amp_damping_probability=amp_damping_probability,
            multi_qubit_gate_factor=multi_qubit_gate_factor,
        )

        counts = sim.simulate(shots=shots)
        end_time = time.time()

        data = ExperimentResultData(
            counts={hex(int(result, 2)): count for result, count in counts.items()},
            statevector=None,
            time_taken=end_time - start_time,
        )

        return ExperimentResult(
            shots=shots,
            success=True,
            status="DONE",
            seed=seed,
            data=data,
            metadata=qc.metadata,
            header=DDSIMHeader.from_quantum_circuit(qc).get_compatible_version(),
        )

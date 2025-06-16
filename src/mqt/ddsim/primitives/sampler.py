# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Sampler implementation using QasmSimulatorBackend."""

from __future__ import annotations

from typing import TYPE_CHECKING

import numpy as np
from mqt.core import load
from qiskit.circuit import QuantumCircuit
from qiskit.primitives.base import BaseSamplerV2
from qiskit.primitives.containers import (
    BitArray,
    DataBin,
    PrimitiveResult,
    SamplerPubLike,
    SamplerPubResult,
)
from qiskit.primitives.containers.sampler_pub import SamplerPub
from qiskit.primitives.primitive_job import PrimitiveJob

from mqt.ddsim.pyddsim import CircuitSimulator

if TYPE_CHECKING:
    from collections.abc import Iterable


class Sampler(BaseSamplerV2):  # type: ignore[misc]
    """DDSIM implementation of Qiskit's sampler.

    The implementation is adapted from Qiskit's `StatevectorSampler`.
    """

    def __init__(self, *, default_shots: int = 1024, seed: np.random.Generator | int | None = -1) -> None:
        """Create a new DDSIM sampler."""
        self._default_shots = default_shots

        if not isinstance(seed, int):
            msg = "seed must be an int."
            raise TypeError(msg)
        self._seed = seed

    @property
    def default_shots(self) -> int:
        """Return the default shots."""
        return self._default_shots

    @property
    def seed(self) -> int:
        """Return the seed."""
        return self._seed

    def run(
        self,
        pubs: Iterable[SamplerPubLike],
        *,
        shots: int | None = None,
    ) -> PrimitiveJob[PrimitiveResult[SamplerPubResult]]:
        """Run and collect samples from each provided PUB (primitive unified bloc).

        Each PUB is run on the CircuitSimulator.
        """
        if shots is None:
            shots = self._default_shots
        coerced_pubs = [SamplerPub.coerce(pub, shots) for pub in pubs]

        job = PrimitiveJob(self._run, coerced_pubs)
        job._submit()  # noqa: SLF001
        return job

    def _run(self, pubs: Iterable[SamplerPub]) -> PrimitiveResult[SamplerPubResult]:
        results = [self._run_pub(pub) for pub in pubs]
        return PrimitiveResult(results, metadata={"version": 2})

    def _run_pub(self, pub: SamplerPub) -> SamplerPubResult:
        circuit = pub.circuit
        parameter_values = pub.parameter_values
        shots = pub.shots

        bound_circuits = parameter_values.bind_all(circuit)
        bound_circuits_list = np.asarray(bound_circuits, dtype=object).tolist()
        if isinstance(bound_circuits_list, QuantumCircuit):
            bound_circuits_list = [bound_circuits_list]

        counts = self._run_experiment(bound_circuits_list, shots=shots, seed=self.seed)

        meas = {creg.name: BitArray.from_counts(counts, creg.size) for creg in circuit.cregs}
        return SamplerPubResult(
            DataBin(**meas, shape=pub.shape),
            metadata={"shots": shots, "circuit_metadata": circuit.metadata},
        )

    @staticmethod
    def _run_experiment(quantum_circuits: list[QuantumCircuit], shots: int, seed: int) -> list[dict[str, int]]:
        counts_list = []

        for quantum_circuit in quantum_circuits:
            loaded_quantum_circuit = load(quantum_circuit)
            simulator = CircuitSimulator(loaded_quantum_circuit, seed=seed)
            counts = simulator.simulate(shots)
            counts_list.append(counts)

        return counts_list

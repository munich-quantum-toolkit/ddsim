# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

from __future__ import annotations

import numpy as np
import pytest
from qiskit import ClassicalRegister, QuantumCircuit, QuantumRegister
from qiskit.circuit import Parameter
from qiskit.primitives import BackendEstimatorV2, BackendSamplerV2
from qiskit.primitives.containers import BindingsArray, EstimatorPub, ObservablesArray, SamplerPub

from mqt.ddsim.primitives import Estimator, Sampler
from mqt.ddsim.qasm_simulator_backend import QasmSimulatorBackend


def test_sampler_broadcast_shots_and_registers() -> None:
    a, b = Parameter("a"), Parameter("b")
    q = QuantumRegister(2)
    left, right = ClassicalRegister(1, "left"), ClassicalRegister(1, "right")
    circuit = QuantumCircuit(q, left, right)
    circuit.ry(b, 0)
    circuit.ry(a, 1)
    circuit.measure(0, right)
    circuit.measure(1, left)
    parameters = np.array([[[0, 0], [0, np.pi]], [[np.pi, 0], [np.pi, np.pi]]])
    sampler = Sampler(default_shots=19, seed=71)
    assert isinstance(sampler, BackendSamplerV2)
    result = sampler.run(
        [
            SamplerPub(circuit, BindingsArray({tuple(circuit.parameters): parameters}), shots=7),
            SamplerPub(circuit, BindingsArray({tuple(circuit.parameters): [0, np.pi]})),
        ],
        shots=11,
    ).result()
    assert result[0].data["left"].shape == (2, 2)
    assert result[0].data["right"].shape == (2, 2)
    assert result[0].data["left"].num_shots == 7
    assert result[1].data["right"].num_shots == 11
    for i in range(2):
        for j in range(2):
            assert result[0].data["left"].get_counts((i, j)) == {str(i): 7}
            assert result[0].data["right"].get_counts((i, j)) == {str(j): 7}


def test_sampler_without_classical_registers() -> None:
    circuit = QuantumCircuit(1)
    circuit.h(0)
    for sampler in [Sampler(), QasmSimulatorBackend().sampler()]:
        with pytest.warns(UserWarning, match="no output classical registers"):
            result = sampler.run([circuit], shots=3).result()
        assert not list(result[0].data.keys())
        assert circuit.num_clbits == 0


def test_exact_estimator_and_sampled_factory() -> None:
    a, b = Parameter("a"), Parameter("b")
    circuit = QuantumCircuit(2)
    circuit.ry(b, 0)
    circuit.ry(a, 1)
    result = (
        Estimator(default_precision=0.1, seed=71)
        .run([
            EstimatorPub(
                circuit,
                ObservablesArray([["IZ"], ["ZI"]]),
                BindingsArray({tuple(circuit.parameters): [[0, 0], [0, np.pi]]}),
                precision=0.2,
            )
        ])
        .result()[0]
    )
    np.testing.assert_allclose(result.data["evs"], [[1, -1], [1, 1]], atol=1e-12)
    np.testing.assert_array_equal(result.data["stds"], np.zeros((2, 2)))
    assert result.metadata["method"] == "exact"
    assert result.metadata["target_precision"] == pytest.approx(0.2)
    assert isinstance(QasmSimulatorBackend().estimator(), BackendEstimatorV2)

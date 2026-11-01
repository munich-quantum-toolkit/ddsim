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
from mqt.core.mlir import QCProgram
from mqt.core.plugins.qiskit import UnsupportedOperationError
from qiskit import ClassicalRegister, QuantumCircuit, QuantumRegister, transpile
from qiskit.circuit import Parameter
from qiskit.quantum_info import Statevector

from mqt.ddsim import build_functionality, sample, simulate
from mqt.ddsim.sampling_simulator_backend import SamplingSimulatorBackend


def test_state_phase_width_and_terminal_readout() -> None:
    circuit = QuantumCircuit(4, 2, global_phase=0.37)
    circuit.x(0)
    circuit.h(2)
    circuit.sdg(2)
    expected = Statevector(circuit).data
    circuit.measure(2, 0)
    circuit.measure(0, 1)
    state = simulate(circuit)
    assert len(state) == 16
    np.testing.assert_allclose(state, expected, atol=1e-12)
    assert not np.allclose(state * np.exp(-0.37j), expected)
    assert not np.allclose(state[::-1], expected)


def test_partial_repeated_and_feedback() -> None:
    q = QuantumRegister(3)
    a = ClassicalRegister(2, "a")
    b = ClassicalRegister(1, "b")
    circuit = QuantumCircuit(q, a, b)
    circuit.x(2)
    circuit.measure(2, a[0])
    with circuit.if_test((a[0], True)):
        circuit.x(0)
    circuit.measure(0, b[0])
    circuit.measure(1, a[0])
    result = SamplingSimulatorBackend().run(circuit, shots=11, memory=True).result()
    assert result.get_counts() == {"1 00": 11}
    assert result.get_memory() == ["1 00"] * 11
    with pytest.raises(ValueError, match="measurement"):
        simulate(circuit)


def test_bound_parameter_order() -> None:
    alpha, beta = Parameter("alpha"), Parameter("beta")
    circuit = QuantumCircuit(2)
    circuit.ry(beta, 0)
    circuit.ry(alpha, 1)
    values = [0.0, np.pi]
    bound = circuit.assign_parameters(values)
    bound.measure_all()
    measured = circuit.copy()
    measured.measure_all()
    result = SamplingSimulatorBackend().run(measured, parameter_values=[values], shots=9).result()
    assert result.get_counts() == {"01": 9}
    expected = np.array([0, 1, 0, 0], dtype=complex)
    np.testing.assert_allclose(simulate(bound), expected, atol=1e-12)
    assert not np.allclose(Statevector(circuit.assign_parameters(values[::-1])).data, expected)
    assert circuit.num_parameters == 2


@pytest.mark.parametrize("seed", [0, 2**31])
def test_invalid_sampling_seeds(seed: object) -> None:
    with pytest.raises(ValueError, match="seed"):
        SamplingSimulatorBackend().run(QuantumCircuit(1), seed_simulator=seed)


def test_serial_seed_reproducibility_and_statistics() -> None:
    circuit = QuantumCircuit(1, 1)
    circuit.h(0)
    circuit.measure(0, 0)
    backend = SamplingSimulatorBackend()
    first = backend.run(circuit, shots=4096, seed_simulator=73, max_workers=1, memory=True).result()
    second = backend.run(circuit, shots=4096, seed_simulator=73, max_workers=1, memory=True).result()
    assert first.get_memory() == second.get_memory()
    # Six binomial standard deviations; either tail has probability below 2e-9.
    assert abs(first.get_counts()["1"] - 2048) < 6 * np.sqrt(4096 / 4)


def test_named_openqasm_inputs() -> None:
    source = 'OPENQASM 3.0; include "stdgates.inc"; input float[64] theta; qubit[2] q; ry(theta) q[0];'
    expected = np.array([0, 1, 0, 0], dtype=complex)
    bound = QCProgram.from_openqasm_str(source)
    bound.bind_parameters({"theta": np.pi})
    np.testing.assert_allclose(simulate(bound), expected, atol=1e-12)
    unitary = build_functionality(bound)
    np.testing.assert_allclose(unitary[:, 0], expected, atol=1e-12)
    measured = source + " bit[2] c; c = measure q;"
    bound_measured = QCProgram.from_openqasm_str(measured)
    bound_measured.bind_parameters({"theta": np.pi})
    assert sample(bound_measured, shots=13) == {"01": 13}
    with pytest.raises(ValueError, match="binding"):
        simulate(source)


def test_invalid_worker_limit() -> None:
    with pytest.raises(ValueError, match="max_workers"):
        SamplingSimulatorBackend().run(QuantumCircuit(1), max_workers=-1)


def test_compact_dd_sampling() -> None:
    circuit = QuantumCircuit(64, 64)
    circuit.h(0)
    for qubit in range(1, 64):
        circuit.cx(0, qubit)
    circuit.measure(range(64), range(64))
    for counts in [
        sample(circuit, shots=32, seed=71),
        SamplingSimulatorBackend().run(circuit, shots=32, seed_simulator=71, max_workers=1).result().get_counts(),
    ]:
        assert set(counts) <= {"0" * 64, "1" * 64}
        assert sum(counts.values()) == 32


def test_composite_controlled_adjoint_requires_transpilation() -> None:
    preparation = QuantumCircuit(1, name="rotation")
    preparation.ry(0.7, 0)
    circuit = QuantumCircuit(2, 2)
    circuit.h(0)
    circuit.x(1)
    circuit.append(preparation.to_gate().control().inverse(), [1, 0])
    circuit.measure(range(2), range(2))
    backend = SamplingSimulatorBackend()
    with pytest.raises(UnsupportedOperationError, match="Unsupported operation"):
        backend.run(circuit, shots=4096)
    native = transpile(circuit, backend, optimization_level=0)
    counts = backend.run(native, shots=4096, seed_simulator=71, max_workers=1).result().get_counts()
    assert set(counts) == {"10", "11"}
    probability = (1 - np.sin(0.7)) / 2
    assert abs(counts["11"] - 4096 * probability) < 6 * np.sqrt(4096 * probability * (1 - probability))

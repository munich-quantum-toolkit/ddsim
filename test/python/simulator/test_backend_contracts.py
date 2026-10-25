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
from qiskit.exceptions import QiskitError
from qiskit.quantum_info import Statevector

from mqt.ddsim import DDSIMProvider, build_functionality, sample, simulate
from mqt.ddsim.qasm_simulator_backend import QasmSimulatorBackend
from mqt.ddsim.statevector_simulator_backend import StatevectorSimulatorBackend


def test_state_phase_width_and_terminal_readout() -> None:
    circuit = QuantumCircuit(4, 2, global_phase=0.37)
    circuit.x(0)
    circuit.h(2)
    circuit.sdg(2)
    expected = Statevector(circuit).data
    circuit.measure(2, 0)
    circuit.measure(0, 1)
    result = StatevectorSimulatorBackend().run(circuit).result()
    state = result.get_statevector()
    assert len(state) == 16
    np.testing.assert_allclose(state, expected, atol=1e-12)
    assert not np.allclose(state * np.exp(-0.37j), expected)
    assert not np.allclose(state[::-1], expected)
    assert "counts" not in result.data()
    assert result.results[0].shots == 0


@pytest.mark.parametrize("options", [{"shots": 10}, {"memory": True}, {"seed_simulator": 7}])
def test_state_has_no_sampling_options(options: dict[str, object]) -> None:
    with pytest.raises(ValueError, match="Dense simulation"):
        StatevectorSimulatorBackend().run(QuantumCircuit(1), **options)  # ty: ignore[invalid-argument-type]


def test_no_classical_bits_is_copied() -> None:
    circuit = QuantumCircuit(3)
    circuit.x(0)
    result = QasmSimulatorBackend().run(circuit, shots=9).result()
    assert result.get_counts() == {"001": 9}
    assert circuit.num_clbits == 0
    assert len(circuit.data) == 1


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
    result = QasmSimulatorBackend().run(circuit, shots=11, memory=True).result()
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
    result = StatevectorSimulatorBackend().run(circuit, parameter_values=[values]).result()
    expected = np.array([0, 1, 0, 0], dtype=complex)
    np.testing.assert_allclose(result.get_statevector(circuit), expected, atol=1e-12)
    assert not np.allclose(Statevector(circuit.assign_parameters(values[::-1])).data, expected)
    assert circuit.num_parameters == 2


@pytest.mark.parametrize("seed", [0, True, -2, 2**31, 1.5])
def test_invalid_sampling_seeds(seed: object) -> None:
    with pytest.raises(ValueError, match="seed"):
        QasmSimulatorBackend().run(QuantumCircuit(1), seed_simulator=seed)
    with pytest.raises(ValueError, match="seed"):
        sample(QuantumCircuit(1), seed=seed)  # ty: ignore[invalid-argument-type]


def test_serial_seed_reproducibility_and_statistics() -> None:
    circuit = QuantumCircuit(1, 1)
    circuit.h(0)
    circuit.measure(0, 0)
    backend = QasmSimulatorBackend()
    first = backend.run(circuit, shots=4096, seed_simulator=73, max_workers=1, memory=True).result()
    second = backend.run(circuit, shots=4096, seed_simulator=73, max_workers=1, memory=True).result()
    assert first.get_memory() == second.get_memory()
    # Six binomial standard deviations; either tail has probability below 2e-9.
    assert abs(first.get_counts()["1"] - 2048) < 6 * np.sqrt(4096 / 4)


def test_removed_backends() -> None:
    assert {backend.name for backend in DDSIMProvider().backends()} == {
        "qasm_simulator",
        "statevector_simulator",
        "unitary_simulator",
    }
    with pytest.raises(QiskitError, match="removed"):
        DDSIMProvider().get_backend("hybrid_qasm_simulator")


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


@pytest.mark.parametrize("workers", [-1, True, 2**31, 0.5])
def test_invalid_worker_limit(workers: object) -> None:
    with pytest.raises(ValueError, match="max_workers"):
        QasmSimulatorBackend().run(QuantumCircuit(1), max_workers=workers)


@pytest.mark.parametrize("shots", [0, -1, True, 1.5])
def test_invalid_shots(shots: object) -> None:
    with pytest.raises(ValueError, match="shots"):
        QasmSimulatorBackend().run(QuantumCircuit(1), shots=shots)
    with pytest.raises(ValueError, match="shots"):
        sample(QuantumCircuit(1), shots=shots)  # ty: ignore[invalid-argument-type]


def test_compact_dd_sampling() -> None:
    circuit = QuantumCircuit(64, 64)
    circuit.h(0)
    for qubit in range(1, 64):
        circuit.cx(0, qubit)
    circuit.measure(range(64), range(64))
    for counts in [
        sample(circuit, shots=32, seed=71),
        QasmSimulatorBackend().run(circuit, shots=32, seed_simulator=71, max_workers=1).result().get_counts(),
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
    backend = QasmSimulatorBackend()
    with pytest.raises(UnsupportedOperationError, match="Unsupported operation"):
        backend.run(circuit, shots=4096)
    native = transpile(circuit, backend, optimization_level=0)
    counts = backend.run(native, shots=4096, seed_simulator=71, max_workers=1).result().get_counts()
    assert set(counts) == {"10", "11"}
    probability = (1 - np.sin(0.7)) / 2
    assert abs(counts["11"] - 4096 * probability) < 6 * np.sqrt(4096 * probability * (1 - probability))


def test_state_backend_cannot_select_unitary_mode() -> None:
    with pytest.raises(TypeError, match="unitary"):
        StatevectorSimulatorBackend(unitary=True)  # ty: ignore[unknown-argument]

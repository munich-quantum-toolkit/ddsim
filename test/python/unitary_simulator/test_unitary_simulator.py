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
from qiskit import QuantumCircuit
from qiskit.quantum_info import Operator

from mqt.ddsim.unitary_simulator_backend import UnitarySimulatorBackend


def test_unitary_phase_order_and_adjoint() -> None:
    circuit = QuantumCircuit(3, global_phase=0.43)
    circuit.h(2)
    circuit.x(0)
    circuit.sdg(1)
    circuit.crx(0.72, 2, 0)
    circuit.cx(0, 1)
    result = UnitarySimulatorBackend().run(circuit).result()
    actual = result.get_unitary()
    expected = Operator(circuit).data
    np.testing.assert_allclose(actual, expected, atol=1e-12)
    assert not np.allclose(actual * np.exp(-0.43j), expected)
    assert circuit.metadata == {}


@pytest.mark.parametrize("operation", ["measure", "reset"])
def test_nonunitary_rejection(operation: str) -> None:
    circuit = QuantumCircuit(1, 1)
    if operation == "measure":
        circuit.measure(0, 0)
    else:
        circuit.reset(0)
    with pytest.raises(ValueError, match=r"[Mm]easure|[Rr]eset|unitary"):
        UnitarySimulatorBackend().run(circuit)


def test_removed_construction_mode() -> None:
    with pytest.raises(ValueError, match="mode"):
        UnitarySimulatorBackend().run(QuantumCircuit(1), mode="recursive")

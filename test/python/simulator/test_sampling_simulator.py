# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

import numpy as np
from qiskit import QuantumCircuit
from qiskit.quantum_info import Statevector

from mqt.ddsim import SamplingSimulatorBackend


def test_portfolio_qaoa() -> None:
    """Check a three-qubit MQT Bench portfolio QAOA program against dense probabilities."""
    circuit = QuantumCircuit.from_qasm_str("""OPENQASM 2.0;
        include "qelib1.inc";
        qreg q[3];
        creg meas[3];
        u2(0.41951949,-pi) q[0];
        u2(0.41620669,-pi) q[1];
        rzz(-0.420917333908502) q[0],q[1];
        u2(0.41905329,-pi) q[2];
        rzz(-0.421016123405307) q[0],q[2];
        u3(2.2348228,1.2558831,-pi/2) q[0];
        rzz(-0.420940441831552) q[1],q[2];
        u3(2.2348228,1.2087537,-pi/2) q[1];
        rzz(-5.98815838177421) q[0],q[1];
        u3(2.2348228,1.2492507,-pi/2) q[2];
        rzz(-5.98956380537088) q[0],q[2];
        u3(0.56042125,-1.2358007,pi/2) q[0];
        rzz(-5.98848712542991) q[1],q[2];
        u3(0.56042125,-1.2880621,pi/2) q[1];
        rzz(-6.64023274758061) q[0],q[1];
        u3(0.56042125,-1.2431553,pi/2) q[2];
        rzz(-6.6417912133385) q[0],q[2];
        rx(-4.06512402388918) q[0];
        rzz(-6.64059728943955) q[1],q[2];
        rx(-4.06512402388918) q[1];
        rx(-4.06512402388918) q[2];
        barrier q[0],q[1],q[2];
        measure q[0] -> meas[0];
        measure q[1] -> meas[1];
        measure q[2] -> meas[2];
        """)
    preparation = circuit.copy()
    preparation.remove_final_measurements()
    probabilities = Statevector(preparation).probabilities_dict()
    shots = 8192
    counts = SamplingSimulatorBackend().run(circuit, shots=shots, seed_simulator=1337).result().get_counts()
    for bits, probability in probabilities.items():
        assert abs(counts.get(bits, 0) - shots * probability) < 6 * np.sqrt(shots * probability * (1 - probability))

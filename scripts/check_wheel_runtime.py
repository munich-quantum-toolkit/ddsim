# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Check an installed MQT DDSIM wheel from outside its source checkout."""

from __future__ import annotations

import json
import sys
from importlib.metadata import distribution, version

import numpy as np
from mqt.core.mlir import OutputFormat, compile_program
from mqt.core.plugins.qiskit import QDMIBackend
from mqt.core.qdmi import ProgramFormat, device_ids
from qiskit import QuantumCircuit
from qiskit.quantum_info import Operator, SparsePauliOp, Statevector

from mqt.ddsim import DDSIMProvider, build_functionality, sample, simulate
from mqt.ddsim.primitives import DDEstimator


def main() -> None:
    """Exercise installed backends, primitives and QIR execution against small oracles."""
    dist = distribution("mqt-ddsim")
    files = [str(path) for path in dist.files or ()]
    assert not any(path.endswith((".so", ".dll", ".dylib", ".pyd")) for path in files)
    circuit = QuantumCircuit(4, global_phase=0.37)
    circuit.h(0)
    circuit.cx(0, 2)
    circuit.sdg(2)
    np.testing.assert_allclose(simulate(circuit), Statevector(circuit).data, atol=2e-12)
    np.testing.assert_allclose(build_functionality(circuit), Operator(circuit).data, atol=2e-12)
    observable = SparsePauliOp.from_list([("IXIY", 0.7), ("IZIZ", -0.2)])
    expected = Statevector(circuit).expectation_value(observable).real
    actual = DDEstimator().run([(circuit, observable)]).result()[0].data["evs"]
    np.testing.assert_allclose(actual, expected, atol=2e-12)

    measured = QuantumCircuit(2)
    measured.x(0)
    measured.measure_all()
    backend = DDSIMProvider().get_backend("sampling_simulator")
    assert isinstance(backend, QDMIBackend)
    assert backend.device_id == "mqt.ddsim.default"
    assert backend.run(measured, shots=16, memory=True).result().get_counts() == {"01": 16}
    assert backend.sampler().run([(measured,)], shots=16).result()[0].data["meas"].get_counts() == {"01": 16}

    # Original source has q0=1, q1=0; output records reverse their measurement map.
    source = (
        'OPENQASM 3.0; include "stdgates.inc"; qubit[2] q; bit[2] c; x q[0]; c[0] = measure q[1]; c[1] = measure q[0];'
    )
    qir = compile_program(source, output=OutputFormat.QIR_BASE)
    assert "mqt.ddsim.default" in device_ids()
    payload = qir.to_bitcode()
    assert sample(payload, shots=5, program_format=ProgramFormat.QIR_BASE_MODULE) == {"10": 5}
    np.testing.assert_array_equal(simulate(payload, program_format=ProgramFormat.QIR_BASE_MODULE), [0, 1, 0, 0])

    sys.stdout.write(
        json.dumps({"ddsim": version("mqt-ddsim"), "core": version("mqt-core"), "status": "passed"}) + "\n"
    )


if __name__ == "__main__":
    main()

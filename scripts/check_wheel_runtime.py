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
import subprocess
import sys
import sysconfig
import tempfile
from importlib.metadata import distribution, version
from pathlib import Path

import numpy as np
from mqt.core.mlir import OutputFormat, compile_program
from mqt.core.plugins.qiskit import QDMIBackend
from mqt.core.qdmi import Job, ProgramFormat, device_ids, open_device
from qiskit import QuantumCircuit
from qiskit.quantum_info import Operator, SparsePauliOp, Statevector

from mqt.ddsim import DDSIMProvider, build_functionality, sample, simulate
from mqt.ddsim.primitives import Estimator, Sampler


def main() -> None:
    """Exercise installed backends, primitives and the CLI against small oracles."""
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
    actual = Estimator().run([(circuit, observable)]).result()[0].data["evs"]
    np.testing.assert_allclose(actual, expected, atol=2e-12)

    measured = QuantumCircuit(2)
    measured.x(0)
    measured.measure_all()
    backend = DDSIMProvider().get_backend("qasm_simulator")
    assert isinstance(backend, QDMIBackend)
    assert backend.device_id == "mqt.ddsim.default"
    assert backend.run(measured, shots=16, memory=True).result().get_counts() == {"01": 16}
    assert Sampler().run([(measured,)], shots=16).result()[0].data["meas"].get_counts() == {"01": 16}

    # Original source has q0=1, q1=0; output records reverse their measurement map.
    source = (
        'OPENQASM 3.0; include "stdgates.inc"; qubit[2] q; bit[2] c; x q[0]; c[0] = measure q[1]; c[1] = measure q[0];'
    )
    qir = compile_program(source, output=OutputFormat.QIR_BASE)
    payloads = [
        (qir.to_llvm_ir(copy=True), ProgramFormat.QIR_BASE_STRING),
        (qir.to_bitcode(), ProgramFormat.QIR_BASE_MODULE),
    ]
    assert "mqt.ddsim.default" in device_ids()
    device = open_device("mqt.ddsim.default")
    for payload, fmt in payloads:
        job = device.submit_job(payload, fmt, num_shots=5, custom1=71, custom3=1)
        assert job.wait()
        assert job.check() == Job.Status.DONE
        assert job.get_counts() == {"10": 5}
        assert job.get_shots() == ["10"] * 5
        assert job.get_program(bytes) == (payload.encode() + b"\0" if isinstance(payload, str) else payload)
        assert sample(payload, shots=5, program_format=fmt) == {"10": 5}
        np.testing.assert_array_equal(simulate(payload, program_format=fmt), [0, 1, 0, 0])

    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "circuit.qasm"
        path.write_text('OPENQASM 3.0; include "stdgates.inc"; qubit[2] q; bit[2] c; x q[0]; c = measure q;')
        executable = Path(sysconfig.get_path("scripts")) / (
            "mqt-ddsim-simple.exe" if sys.platform == "win32" else "mqt-ddsim-simple"
        )
        completed = subprocess.run(  # ruff: ignore[subprocess-without-shell-equals-true] - invokes the installed CLI
            [str(executable), "--simulate_file", str(path), "--shots", "16"],
            cwd=directory,
            check=True,
            capture_output=True,
            text=True,
        )
        assert json.loads(completed.stdout)["counts"] == {"01": 16}
    sys.stdout.write(
        json.dumps({"ddsim": version("mqt-ddsim"), "core": version("mqt-core"), "status": "passed"}) + "\n"
    )


if __name__ == "__main__":
    main()

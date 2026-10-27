# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Public simulation functions and explicit QIR execution."""

from __future__ import annotations

from numbers import Integral
from typing import TYPE_CHECKING, cast

import numpy as np

from mqt.core import mlir
from mqt.core.mlir import build_functionality
from mqt.core.qdmi import Job, ProgramFormat, open_device

if TYPE_CHECKING:
    from os import PathLike

    from numpy.typing import NDArray
    from qiskit import QuantumCircuit

    from mqt.core.mlir import JeffProgram, OpenQASMProgram, QCOProgram, QCProgram

    Program = str | PathLike[str] | QuantumCircuit | QCProgram | QCOProgram | OpenQASMProgram | JeffProgram
_QIR_PAYLOAD_TYPES = {
    ProgramFormat.QIR_BASE_STRING: str,
    ProgramFormat.QIR_ADAPTIVE_STRING: str,
    ProgramFormat.QIR_BASE_MODULE: bytes,
    ProgramFormat.QIR_ADAPTIVE_MODULE: bytes,
}

__all__ = ["build_functionality", "sample", "simulate"]


def _seed(seed: object) -> int | None:
    if seed is None:
        return None
    if isinstance(seed, bool) or not isinstance(seed, Integral) or seed not in range(1, 2**31):
        msg = "seed must be None or an integer from 1 to 2147483647"
        raise ValueError(msg)
    return int(seed)


def _shots(shots: object) -> int:
    if isinstance(shots, bool) or not isinstance(shots, Integral) or shots <= 0:
        msg = "shots must be a positive integer"
        raise ValueError(msg)
    return int(shots)


def _qir_job(program: str | bytes, program_format: ProgramFormat, shots: int, seed: int | None = None) -> Job:
    expected_type = _QIR_PAYLOAD_TYPES.get(program_format)
    if expected_type is None:
        msg = "program_format must select QIR Base or Adaptive text/bitcode"
        raise ValueError(msg)
    if not isinstance(program, expected_type):
        msg = f"{program_format.name} requires {expected_type.__name__} input"
        raise TypeError(msg)
    job = open_device("mqt.ddsim.default").submit_job(program, program_format, num_shots=shots, custom1=seed)
    job.wait()
    if job.check() != Job.Status.DONE:
        msg = "MQT DDSIM QIR execution failed; verify the declared QIR profile and supported operations"
        raise RuntimeError(msg)
    return job


def simulate(
    program: Program | bytes,
    *,
    program_format: ProgramFormat | None = None,
) -> NDArray[np.complex128]:
    """Return the uncollapsed statevector from the all-zero state.

    Args:
        program: A Qiskit circuit, OpenQASM source/path, or QIR payload.
        program_format: Required for QIR; text requires str and bitcode requires bytes.

    Terminal measurements do not collapse the state. Reset and measurement-dependent
    feedback are unsupported. The array uses qubit zero as its least-significant bit.
    QIR profile declarations are caller guarantees, not conformance validation.
    """
    if program_format is not None:
        # Read raw QDMI DENSE_STATEVECTOR bytes without building a Python complex list.
        return np.frombuffer(
            _qir_job(cast("str | bytes", program), program_format, 0).get_results(3), dtype=np.complex128
        )
    return mlir.simulate(cast("Program", program))


def sample(
    program: Program | bytes,
    *,
    shots: int = 1024,
    seed: int | None = None,
    program_format: ProgramFormat | None = None,
) -> dict[str, int]:
    """Sample a Qiskit, OpenQASM, or explicitly declared QIR program.

    Args:
        program: A bound Qiskit circuit, OpenQASM source/path, or QIR payload.
        shots: Positive number of samples.
        seed: None for randomness; otherwise a positive signed 32-bit integer.
        program_format: Required for QIR; text requires str and bitcode requires bytes.

    Seeds reproduce samples only for the same execution path, program, build, and
    worker configuration. QIR counts follow QDMI output-record ordering.
    """
    shots = _shots(shots)
    seed = _seed(seed)
    if program_format is not None:
        return _qir_job(cast("str | bytes", program), program_format, shots, seed).get_counts()
    return mlir.sample(cast("Program", program), shots=shots, seed=seed or 0)

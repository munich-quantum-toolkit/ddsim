# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest
from mqt.core.qdmi import ProgramFormat

from mqt.ddsim import sample, simulate

ROOT = Path(__file__).parent


@pytest.mark.parametrize("bitcode", [False, True])
def test_base_order_and_terminal_state(bitcode: bool) -> None:
    program = (ROOT / "asymmetric.bc").read_bytes() if bitcode else (ROOT / "asymmetric.ll").read_text()
    fmt = ProgramFormat.QIR_BASE_MODULE if bitcode else ProgramFormat.QIR_BASE_STRING
    assert sample(program, shots=17, program_format=fmt) == {"011": 17}
    expected = np.zeros(8, dtype=complex)
    expected[3] = 1
    np.testing.assert_array_equal(simulate(program, program_format=fmt), expected)
    assert not np.array_equal(expected, expected[::-1])


@pytest.mark.parametrize("bitcode", [False, True])
def test_adaptive_feedback_and_reset(bitcode: bool) -> None:
    program = (ROOT / "feedback.bc").read_bytes() if bitcode else (ROOT / "feedback.ll").read_text()
    fmt = ProgramFormat.QIR_ADAPTIVE_MODULE if bitcode else ProgramFormat.QIR_ADAPTIVE_STRING
    counts = sample(program, shots=1024, seed=71, program_format=fmt)
    assert set(counts) == {"000", "011"}
    assert abs(counts["011"] - 512) < 6 * np.sqrt(1024 / 4)
    with pytest.raises(RuntimeError, match="QIR execution failed"):
        simulate(program, program_format=fmt)


def test_qir_control_adjoint_global_phase() -> None:
    program = (ROOT / "phase.ll").read_text()
    actual = simulate(program, program_format=ProgramFormat.QIR_BASE_STRING)
    expected = np.exp(0.37j) * np.array([0, 0, 1, -1j]) / np.sqrt(2)
    np.testing.assert_allclose(actual, expected, atol=1e-12)
    assert not np.allclose(actual * np.exp(-0.37j), expected)


@pytest.mark.parametrize(
    ("program", "fmt"), [(b"bitcode", ProgramFormat.QIR_BASE_STRING), ("text", ProgramFormat.QIR_BASE_MODULE)]
)
def test_payload_type(program: str | bytes, fmt: ProgramFormat) -> None:
    with pytest.raises(TypeError, match="requires"):
        sample(program, program_format=fmt)

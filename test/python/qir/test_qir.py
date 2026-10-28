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
from mqt.core.mlir import OutputFormat, QIRProgram, compile_program
from mqt.core.qdmi import ProgramFormat

from mqt.ddsim import sample, simulate


@pytest.mark.parametrize("output", [OutputFormat.QIR_BASE, OutputFormat.QIR_ADAPTIVE])
def test_qir_program_and_payload(output: OutputFormat) -> None:
    source = 'OPENQASM 3; include "stdgates.inc"; qubit[2] q; bit[2] c; x q[0]; c = measure q;'
    program = compile_program(source, output=output)
    assert isinstance(program, QIRProgram)
    assert sample(program, shots=17) == {"01": 17}
    np.testing.assert_array_equal(simulate(program), [0, 1, 0, 0])
    fmt = ProgramFormat.QIR_BASE_MODULE if output == OutputFormat.QIR_BASE else ProgramFormat.QIR_ADAPTIVE_MODULE
    assert sample(program.to_bitcode(), shots=17, program_format=fmt) == {"01": 17}

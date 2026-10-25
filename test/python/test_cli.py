# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Installed command behavior and JSON contracts."""

from __future__ import annotations

import json
import subprocess
import sys

import numpy as np
import pytest


@pytest.mark.parametrize(("mode", "shape"), [("sample", None), ("statevector", [4]), ("unitary", [4, 4])])
def test_modes(tmp_path, mode, shape):
    program = tmp_path / "program.qasm"
    program.write_text('OPENQASM 3.0; include "stdgates.inc"; qubit[2] q; x q[0]; gphase(0.37);')
    command = [sys.executable, "-m", "mqt.ddsim.cli", "--simulate_file", str(program), "--mode", mode, "--ps"]
    if mode == "sample":
        command.extend(["--shots", "16", "--seed", "17"])
    result = subprocess.run(command, cwd=tmp_path, capture_output=True, text=True, check=True)  # ruff: ignore[subprocess-without-shell-equals-true]
    data = json.loads(result.stdout)
    assert data["schema_version"] == 1
    assert data["mode"] == mode
    assert data["statistics"]["input"] == str(program)
    if mode == "sample":
        assert data["counts"] == {"01": 16}
    else:
        assert data["shape"] == shape
        array = np.asarray(data["data"])
        array = array[..., 0] + 1j * array[..., 1]
        phase = np.exp(0.37j)
        expected = (
            np.array([0, phase, 0, 0])
            if mode == "statevector"
            else phase * np.array([[0, 1, 0, 0], [1, 0, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0]])
        )
        np.testing.assert_allclose(array, expected, atol=2e-12)


@pytest.mark.parametrize("options", [["--pv", "--shots", "1"], ["--mode", "unitary", "--seed", "1"], ["--hybrid"]])
def test_invalid_options(tmp_path, options):
    result = subprocess.run(  # ruff: ignore[subprocess-without-shell-equals-true] - invokes this interpreter and the package under test
        [sys.executable, "-m", "mqt.ddsim.cli", "--simulate_file", str(tmp_path / "missing.qasm"), *options],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 2
    assert not result.stdout


def test_execution_error(tmp_path):
    result = subprocess.run(  # ruff: ignore[subprocess-without-shell-equals-true] - invokes this interpreter and the package under test
        [sys.executable, "-m", "mqt.ddsim.cli", "--simulate_file", str(tmp_path / "missing.qasm")],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 1
    assert "error" in json.loads(result.stderr)

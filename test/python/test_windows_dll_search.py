# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Windows DLL search path regression test."""

from __future__ import annotations

import sys
import sysconfig
from pathlib import Path

import pytest


@pytest.mark.skipif(sys.platform != "win32", reason="Windows DLL search path only")
def test_core_dll_directory_in_active_environment() -> None:
    """Check that uv has not redirected sysconfig to a temporary overlay."""
    assert (Path(sysconfig.get_paths()["purelib"]) / "mqt" / "core" / "bin").is_dir()

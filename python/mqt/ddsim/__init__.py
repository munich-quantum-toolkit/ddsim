# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM Python Package."""

from __future__ import annotations

import sys

# under Windows, make sure to add the appropriate DLL directory to the PATH
if sys.platform == "win32":  # ruff:ignore[non-empty-init-module] This is actually required on Windows

    def _dll_patch() -> None:
        """Add the DLL directory to the PATH."""
        import os  # ruff:ignore[import-outside-top-level] because only needed on Windows
        import sysconfig  # ruff:ignore[import-outside-top-level] because only needed on Windows
        from pathlib import Path  # ruff:ignore[import-outside-top-level] because only needed on Windows

        site_packages = Path(sysconfig.get_paths()["purelib"])
        bin_dir = site_packages / "mqt" / "core" / "bin"
        os.add_dll_directory(str(bin_dir))

    _dll_patch()
    del _dll_patch

from ._simulation import sample, simulate_statevector
from ._version import version as __version__
from .provider import DDSIMProvider
from .pyddsim import (
    CircuitSimulator,
    DeterministicNoiseSimulator,
    HybridSimulator,
    HybridSimulatorMode,
    PathSimulator,
    PathSimulatorConfiguration,
    PathSimulatorMode,
    StochasticNoiseSimulator,
    UnitarySimulator,
    UnitarySimulatorMode,
)

__all__ = [
    "CircuitSimulator",
    "DDSIMProvider",
    "DeterministicNoiseSimulator",
    "HybridSimulator",
    "HybridSimulatorMode",
    "PathSimulator",
    "PathSimulatorConfiguration",
    "PathSimulatorMode",
    "StochasticNoiseSimulator",
    "UnitarySimulator",
    "UnitarySimulatorMode",
    "__version__",
    "sample",
    "simulate_statevector",
]

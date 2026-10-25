# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM simulation, powered by MQT Core."""

from importlib.metadata import version
from typing import NoReturn

from .provider import DDSIMProvider
from .simulation import build_functionality, sample, simulate

__version__ = version("mqt-ddsim")
__all__ = ["DDSIMProvider", "__version__", "build_functionality", "sample", "simulate"]


def __getattr__(name: str) -> NoReturn:
    """Explain replacements for removed low-level simulator classes."""
    if name in {"CircuitSimulator", "UnitarySimulator", "UnitarySimulatorMode"}:
        msg = f"{name} was removed in MQT DDSIM 3. Use simulate(), sample(), or build_functionality(); use mqt.core.dd for DD operations."
    elif name in {
        "DeterministicNoiseSimulator",
        "HybridSimulator",
        "HybridSimulatorMode",
        "PathSimulator",
        "PathSimulatorConfiguration",
        "PathSimulatorMode",
        "StochasticNoiseSimulator",
    }:
        msg = f"{name} was removed in MQT DDSIM 3. HSF, path, and noise simulation are available only in MQT DDSIM 2.x."
    else:
        msg = f"module {__name__!r} has no attribute {name!r}"
    raise AttributeError(msg)

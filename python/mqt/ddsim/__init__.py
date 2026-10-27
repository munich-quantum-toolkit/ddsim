# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""A quantum circuit simulator based on decision diagrams powered by MQT Core."""

from importlib.metadata import version

from .provider import DDSIMProvider
from .sampling_simulator_backend import SamplingSimulatorBackend
from .simulation import build_functionality, sample, simulate

__version__ = version("mqt-ddsim")
__all__ = ["DDSIMProvider", "SamplingSimulatorBackend", "__version__", "build_functionality", "sample", "simulate"]

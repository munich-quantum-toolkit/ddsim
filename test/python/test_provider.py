# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

from __future__ import annotations

import unittest

import pytest

import mqt.ddsim
from mqt.ddsim import DDSIMProvider


class DDSIMProviderTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.provider = DDSIMProvider()
        self.backend_name = "qasm_simulator"

    def test_backends(self) -> None:
        """Test the provider has backends."""
        backends = self.provider.backends()
        assert len(backends) > 0

    def test_get_backend(self) -> None:
        """Test getting a backend from the provider."""
        backend = self.provider.get_backend(name=self.backend_name)
        assert backend.name == self.backend_name


@pytest.mark.parametrize(
    "name", ["CircuitSimulator", "UnitarySimulator", "HybridSimulator", "PathSimulator", "StochasticNoiseSimulator"]
)
def test_removed_low_level_classes(name: str) -> None:
    with pytest.raises(AttributeError, match="removed in MQT DDSIM 3"):
        getattr(mqt.ddsim, name)

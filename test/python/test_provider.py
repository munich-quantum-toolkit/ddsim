# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

from mqt.ddsim import DDSIMProvider, SamplingSimulatorBackend


def test_provider() -> None:
    provider = DDSIMProvider()
    assert isinstance(provider.get_backend("sampling_simulator"), SamplingSimulatorBackend)
    assert [backend.name for backend in provider.backends()] == ["sampling_simulator"]
    assert provider.backends(name="unknown") == []

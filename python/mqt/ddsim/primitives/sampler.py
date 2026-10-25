# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""MQT DDSIM compatibility constructor for Qiskit's native V2 sampler."""

from qiskit.primitives import BackendSamplerV2

from mqt.ddsim.qasm_simulator_backend import QasmSimulatorBackend
from mqt.ddsim.simulation import _seed, _shots


class Sampler(BackendSamplerV2):
    """Sample PUBs with native Qiskit broadcasting and separate register BitArrays."""

    def __init__(self, *, default_shots: int = 1024, seed: int | None = -1) -> None:
        """Create a sampler with positive shots and an optional positive seed."""
        super().__init__(
            backend=QasmSimulatorBackend(),
            options={"default_shots": _shots(default_shots), "seed_simulator": _seed(seed)},
        )

    @property
    def default_shots(self) -> int:
        """Default number of samples for PUBs without a shot override."""
        return self.options.default_shots

    @property
    def seed(self) -> int | None:
        """Sampling seed; None selects nondeterministic execution."""
        return self.options.seed_simulator

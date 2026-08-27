# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""High-level decision-diagram simulation helpers."""

from __future__ import annotations

from typing import TYPE_CHECKING

from mqt.core.dd import DDPackage
from mqt.core.dd import simulate as simulate_dd

from .pyddsim import CircuitSimulator

if TYPE_CHECKING:
    import numpy as np
    from mqt.core.ir import QuantumComputation
    from numpy.typing import NDArray

_MAX_SEED = (1 << 63) - 1


def sample(qc: QuantumComputation, shots: int = 1024, seed: int = 0) -> dict[str, int]:
    """Sample from the output distribution of a quantum computation.

    Circuits with only terminal measurements are executed once. Circuits with
    mid-circuit measurements, resets, or classical control are executed once
    per shot.

    Args:
        qc: The quantum computation to simulate.
        shots: The number of samples to draw.
        seed: The non-negative signed 64-bit random seed. Zero selects a random
            seed for compatibility with the former `mqt.core.dd.sample` helper.

    Returns:
        A histogram mapping big-endian measurement bitstrings to counts.
    """
    if not 0 <= seed <= _MAX_SEED:
        msg = f"seed must be between 0 and {_MAX_SEED}"
        raise ValueError(msg)

    simulator = CircuitSimulator(qc, seed=-1 if seed == 0 else seed)
    return simulator.simulate(shots)


def simulate_statevector(qc: QuantumComputation) -> NDArray[np.complex128]:
    """Simulate a unitary quantum computation and return its state vector.

    Args:
        qc: The unitary quantum computation to simulate.

    Returns:
        A copy of the final state vector.
    """
    package = DDPackage(qc.num_qubits)
    final_state = simulate_dd(qc, package.zero_state(qc.num_qubits), package)
    try:
        return final_state.get_vector()
    finally:
        package.dec_ref_vec(final_state)

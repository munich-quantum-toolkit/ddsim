# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

import enum
from collections.abc import Sequence
from typing import Any, overload

import mqt.core.dd
import mqt.core.ir

class CircuitSimulator:
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        approximation_step_fidelity: float = 1.0,
        approximation_steps: int = 1,
        approximation_strategy: str = "fidelity",
        seed: int = -1,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def simulate(self, shots: int) -> dict[str, int]:
        """Simulate the circuit and return the result as a dictionary of counts."""

    def get_constructed_dd(self) -> mqt.core.dd.VectorDD:
        """Get the vector DD resulting from the simulation."""

    def expectation_value(self, observable: mqt.core.ir.QuantumComputation) -> float:
        """Compute the expectation value for the given observable."""

class StochasticNoiseSimulator:
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        approximation_step_fidelity: float = 1.0,
        approximation_steps: int = 1,
        approximation_strategy: str = "fidelity",
        seed: int = -1,
        noise_effects: str = "APD",
        noise_probability: float = 0.01,
        amp_damping_probability: float = 0.02,
        multi_qubit_gate_factor: float = 2,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def simulate(self, shots: int) -> dict[str, int]:
        """Simulate the circuit and return the result as a dictionary of counts."""

    def get_constructed_dd(self) -> mqt.core.dd.VectorDD:
        """Get the vector DD resulting from the simulation."""

class DeterministicNoiseSimulator:
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        approximation_step_fidelity: float = 1.0,
        approximation_steps: int = 1,
        approximation_strategy: str = "fidelity",
        seed: int = -1,
        noise_effects: str = "APD",
        noise_probability: float = 0.01,
        amp_damping_probability: float = 0.02,
        multi_qubit_gate_factor: float = 2,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def simulate(self, shots: int) -> dict[str, int]:
        """Simulate the circuit and return the result as a dictionary of counts."""

    def get_constructed_dd(self) -> mqt.core.dd.VectorDD:
        """Get the vector DD resulting from the simulation."""

class HybridSimulatorMode(enum.Enum):
    """Enumeration of modes for the :class:`~HybridSimulator`."""

    DD = 0

    amplitude = 1

class HybridSimulator:
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        approximation_step_fidelity: float = 1.0,
        approximation_steps: int = 1,
        approximation_strategy: str = "fidelity",
        seed: int = -1,
        mode: HybridSimulatorMode = ...,
        nthreads: int = 2,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def simulate(self, shots: int) -> dict[str, int]:
        """Simulate the circuit and return the result as a dictionary of counts."""

    def get_constructed_dd(self) -> mqt.core.dd.VectorDD:
        """Get the vector DD resulting from the simulation."""

    def get_mode(self) -> HybridSimulatorMode:
        """Get the mode of the hybrid simulator."""

    def get_final_amplitudes(self) -> list[complex]:
        """Get the final amplitudes from the hybrid simulation."""

class PathSimulatorMode(enum.Enum):
    """Enumeration of modes for the :class:`~PathSimulator`."""

    sequential = 0

    pairwise_recursive = 1

    bracket = 2

    alternating = 3

    gate_cost = 4

class PathSimulatorConfiguration:
    """Configuration options for the :class:`~.PathSimulator`."""

    def __init__(self) -> None: ...
    @property
    def mode(self) -> PathSimulatorMode:
        """The mode used for determining a simulation path."""

    @mode.setter
    def mode(self, arg: PathSimulatorMode, /) -> None: ...
    @property
    def bracket_size(self) -> int:
        """Size of the brackets one wants to combine."""

    @bracket_size.setter
    def bracket_size(self, arg: int, /) -> None: ...
    @property
    def starting_point(self) -> int:
        """Start of the alternating or gate_cost strategy."""

    @starting_point.setter
    def starting_point(self, arg: int, /) -> None: ...
    @property
    def gate_cost(self) -> list[int]:
        """A list that contains the number of gates which are considered in each step."""

    @gate_cost.setter
    def gate_cost(self, arg: Sequence[int], /) -> None: ...
    @property
    def seed(self) -> int:
        """Seed for the simulator."""

    @seed.setter
    def seed(self, arg: int, /) -> None: ...
    def json(self) -> dict[str, Any]:
        """Get the configuration as a JSON-style dictionary."""

class PathSimulator:
    @overload
    def __init__(self, circ: mqt.core.ir.QuantumComputation, config: PathSimulatorConfiguration = ...) -> None: ...
    @overload
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        mode: PathSimulatorMode = ...,
        bracket_size: int = 2,
        starting_point: int = 0,
        gate_cost: Sequence[int] = [],
        seed: int = 0,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def simulate(self, shots: int) -> dict[str, int]:
        """Simulate the circuit and return the result as a dictionary of counts."""

    def get_constructed_dd(self) -> mqt.core.dd.VectorDD:
        """Get the vector DD resulting from the simulation."""

    def set_simulation_path(self, path: Sequence[tuple[int, int]], assume_correct_order: bool = False) -> None:
        """Set the simulation path.

        Args:
            path: The components of the simulation path.
            assume_correct_order: Whether the provided path is assumed to be in the correct order. Defaults to False.
        """

class UnitarySimulatorMode(enum.Enum):
    """Enumeration of modes for the :class:`~UnitarySimulator`."""

    recursive = 1

    sequential = 0

class UnitarySimulator:
    def __init__(
        self,
        circ: mqt.core.ir.QuantumComputation,
        approximation_step_fidelity: float = 1.0,
        approximation_steps: int = 1,
        approximation_strategy: str = "fidelity",
        seed: int = -1,
        mode: UnitarySimulatorMode = ...,
    ) -> None: ...
    def get_number_of_qubits(self) -> int:
        """Get the number of qubits."""

    def get_name(self) -> str:
        """Get the name of the simulator."""

    def statistics(self) -> dict[str, str]:
        """Get additional statistics provided by the simulator."""

    def get_active_vector_node_count(self) -> int:
        """Get the number of active vector nodes, i.e., the number of vector DD nodes in the unique table with a non-zero reference count."""

    def get_active_matrix_node_count(self) -> int:
        """Get the number of active matrix nodes, i.e., the number of matrix DD nodes in the unique table with a non-zero reference count."""

    def get_tolerance(self) -> float:
        """Get the tolerance for the DD package."""

    def set_tolerance(self, tol: float) -> None:
        """Set the tolerance for the DD package."""

    def construct(self) -> None:
        """Construct the DD representing the unitary matrix of the circuit."""

    def get_mode(self) -> UnitarySimulatorMode:
        """Get the mode of the unitary simulator."""

    def get_construction_time(self) -> float:
        """Get the time taken to construct the DD."""

    def get_final_node_count(self) -> int:
        """Get the final node count of the constructed DD."""

    def get_constructed_dd(self) -> mqt.core.dd.MatrixDD:
        """Get the constructed DD."""

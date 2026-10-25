# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Provider for MQT DDSIM backends."""

from __future__ import annotations

from typing import TYPE_CHECKING, Any

from qiskit.providers.exceptions import QiskitBackendNotFoundError
from qiskit.providers.providerutils import filter_backends

from .qasm_simulator_backend import QasmSimulatorBackend
from .statevector_simulator_backend import StatevectorSimulatorBackend
from .unitary_simulator_backend import UnitarySimulatorBackend

if TYPE_CHECKING:
    from collections.abc import Callable

    from qiskit.providers import Backend, BackendV2


class DDSIMProvider:
    """Provider for MQT DDSIM backends."""

    _BACKENDS: tuple[tuple[str, type[BackendV2]], ...] = (
        ("qasm_simulator", QasmSimulatorBackend),
        ("statevector_simulator", StatevectorSimulatorBackend),
        ("unitary_simulator", UnitarySimulatorBackend),
    )

    def get_backend(self, name: str | None = None, **kwargs: Any) -> BackendV2:
        """Return a backend matching the specified criteria.

        Args:
            name: Name of the backend.
            kwargs: Additional filtering criteria.
        """
        backends = self.backends(name, **kwargs)
        if len(backends) > 1:
            msg = "More than one backend matches the criteria"
            raise QiskitBackendNotFoundError(msg)
        if not backends:
            msg = (
                f"No backend matches {name!r}. MQT DDSIM provides qasm_simulator, statevector_simulator, "
                "and unitary_simulator. HSF, path, and noise backends were removed; use MQT DDSIM 2.x for them."
            )
            raise QiskitBackendNotFoundError(msg)

        return backends[0]

    def backends(
        self,
        name: str | None = None,
        filters: Callable[[Backend], bool] | None = None,
        **kwargs: Any,
    ) -> list[BackendV2]:
        """Return a list of backends matching the specified criteria.

        Args:
            name: Name of the backend.
            filters: Additional filtering criteria.
            kwargs: Additional filtering criteria.
        """
        backends: list[Backend] = [
            backend_cls() for backend_name, backend_cls in self._BACKENDS if name is None or backend_name == name
        ]
        return filter_backends(backends, filters=filters, **kwargs)  # ty: ignore[invalid-return-type]

    def __str__(self) -> str:
        """Return the provider name."""
        return "DDSIMProvider"

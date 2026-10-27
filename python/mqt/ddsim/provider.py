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

from .sampling_simulator_backend import SamplingSimulatorBackend

if TYPE_CHECKING:
    from collections.abc import Callable

    from qiskit.providers import Backend, BackendV2


class DDSIMProvider:
    """Provider for MQT DDSIM backends."""

    def get_backend(self, name: str | None = None, **kwargs: Any) -> BackendV2:
        """Return a backend matching the specified criteria.

        Args:
            name: Name of the backend.
            kwargs: Additional filtering criteria.
        """
        backends = self.backends(name, **kwargs)
        if not backends:
            msg = f"No backend matches {name!r}. Available backend: sampling_simulator."
            raise QiskitBackendNotFoundError(msg)

        return backends[0]

    @staticmethod
    def backends(
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
        backends: list[Backend] = [SamplingSimulatorBackend()] if name in {None, "sampling_simulator"} else []
        return filter_backends(backends, filters=filters, **kwargs)  # ty: ignore[invalid-return-type]

    def __str__(self) -> str:
        """Return the provider name."""
        return "DDSIMProvider"

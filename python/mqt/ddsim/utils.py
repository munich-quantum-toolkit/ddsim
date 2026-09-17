# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Utilities for documentation examples."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from mqt.core.dd import MatrixDD, VectorDD


def to_svg(dd: VectorDD | MatrixDD, filename: str, **options: bool) -> None:
    """Render a decision diagram as SVG with PyGraphviz.

    Remove once the minimum MQT Core version includes PyGraphviz-backed `to_svg()`.
    See https://github.com/munich-quantum-toolkit/ddsim/issues/1013.

    Args:
        dd: Decision diagram to render.
        filename: Output SVG filename.
        **options: Options passed to `dd.to_dot()`.
    """
    # PyGraphviz is an optional documentation dependency.
    from pygraphviz import AGraph  # ruff: ignore[import-outside-top-level] # ty: ignore[unresolved-import]

    AGraph(dd.to_dot(**options)).draw(filename, prog="dot", format="svg")

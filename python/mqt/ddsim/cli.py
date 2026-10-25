# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Command-line entry point for MQT DDSIM."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from time import perf_counter
from typing import TYPE_CHECKING

from mqt.core.qdmi import ProgramFormat

from . import build_functionality, sample, simulate

if TYPE_CHECKING:
    from collections.abc import Sequence


def main(argv: Sequence[str] | None = None) -> int:
    """Run a program and print a versioned JSON result.

    Args:
        argv: Arguments excluding the executable name; defaults to sys.argv.

    Returns:
        Zero on success, one for execution errors, or two for invalid arguments.
    """
    parser = argparse.ArgumentParser(
        description="MQT DDSIM quantum simulation powered by MQT Core.",
        epilog="HSF, path, noise, approximation and algorithm-generator flags were removed; use MQT DDSIM 2.x for them.",
    )
    parser.add_argument("--simulate_file", required=True, type=Path, help="OpenQASM or QIR program")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--mode", choices=("sample", "statevector", "unitary"))
    modes.add_argument("--pm", action="store_const", const="sample", dest="mode", help="return counts")
    modes.add_argument(
        "--pv", action="store_const", const="statevector", dest="mode", help="return an uncollapsed state"
    )
    parser.add_argument("--input-format", choices=("openqasm", "qir-base", "qir-adaptive"), default="openqasm")
    parser.add_argument("--shots", type=int, help="sample count (default: 1024)")
    parser.add_argument("--seed", type=int, help="positive sampling seed, or -1 for nondeterministic sampling")
    parser.add_argument("--ps", action="store_true", help="include elapsed time and input metadata")
    args = parser.parse_args(argv)
    mode = args.mode or "sample"
    if mode != "sample" and (args.shots is not None or args.seed is not None):
        parser.error("--shots and --seed apply only to sampling")
    qir = args.input_format != "openqasm"
    if qir and args.simulate_file.suffix.lower() not in {".ll", ".bc"}:
        parser.error("QIR input requires a .ll text file or a .bc bitcode file")
    if not qir and args.simulate_file.suffix.lower() in {".ll", ".bc"}:
        parser.error("QIR input requires --input-format qir-base or qir-adaptive")
    if qir and mode == "unitary":
        parser.error("full unitary simulation of QIR is unsupported")

    try:
        result = _execute(args, mode, qir=qir)
        sys.stdout.write(json.dumps(result, allow_nan=False) + "\n")
    except (ValueError, RuntimeError, OSError, MemoryError) as error:
        sys.stderr.write(json.dumps({"schema_version": 1, "error": str(error)}) + "\n")
        return 1
    return 0


def _execute(args: argparse.Namespace, mode: str, *, qir: bool) -> dict[str, object]:
    start = perf_counter()
    program: Path | str | bytes = args.simulate_file
    program_format = None
    if qir:
        bitcode = args.simulate_file.suffix.lower() == ".bc"
        program = args.simulate_file.read_bytes() if bitcode else args.simulate_file.read_text(encoding="utf-8")
        formats = {
            ("qir-base", False): ProgramFormat.QIR_BASE_STRING,
            ("qir-base", True): ProgramFormat.QIR_BASE_MODULE,
            ("qir-adaptive", False): ProgramFormat.QIR_ADAPTIVE_STRING,
            ("qir-adaptive", True): ProgramFormat.QIR_ADAPTIVE_MODULE,
        }
        program_format = formats[args.input_format, bitcode]
    result: dict[str, object] = {"schema_version": 1, "mode": mode}
    if mode == "sample":
        shots = args.shots if args.shots is not None else 1024
        result.update(counts=sample(program, shots=shots, seed=args.seed, program_format=program_format), shots=shots)
    else:
        values = (
            simulate(program, program_format=program_format) if mode == "statevector" else build_functionality(program)
        )
        result.update(
            shape=list(values.shape),
            complex_encoding="[real, imaginary]",
            data=[[[z.real, z.imag] for z in row] for row in values]
            if mode == "unitary"
            else [[z.real, z.imag] for z in values],
        )
    if args.ps:
        result["statistics"] = {
            "elapsed_seconds": perf_counter() - start,
            "input": str(args.simulate_file),
            "input_format": args.input_format,
        }
    return result


if __name__ == "__main__":
    sys.exit(main())

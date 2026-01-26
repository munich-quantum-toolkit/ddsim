#!/usr/bin/env -S uv run --script --quiet
# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

# /// script
# dependencies = ["nox"]
# ///


"""Nox sessions."""

from __future__ import annotations

import argparse
import contextlib
import os
import shutil
import sys
import tempfile
from pathlib import Path
from typing import TYPE_CHECKING

import nox

if TYPE_CHECKING:
    from collections.abc import Generator, Sequence

nox.needs_version = ">=2025.10.16"
nox.options.default_venv_backend = "uv"


PYTHON_ALL_VERSIONS = ["3.10", "3.11", "3.12", "3.13", "3.14"]

if os.environ.get("CI", None):
    nox.options.error_on_missing_interpreters = True


@contextlib.contextmanager
def preserve_lockfile() -> Generator[None]:
    """Preserve the lockfile by moving it to a temporary directory."""
    with tempfile.TemporaryDirectory() as temp_dir_name:
        shutil.move("uv.lock", f"{temp_dir_name}/uv.lock")
        try:
            yield
        finally:
            shutil.move(f"{temp_dir_name}/uv.lock", "uv.lock")


@nox.session(reuse_venv=True, default=True)
def lint(session: nox.Session) -> None:
    """Run the linter."""
    if shutil.which("prek") is None:
        session.install("prek")

    session.run("prek", "run", "--all-files", *session.posargs, external=True)


def _run_tests(
    session: nox.Session,
    *,
    install_args: Sequence[str] = (),
    extra_command: Sequence[str] = (),
    run_args: Sequence[str] = (),
) -> None:
    env = {"UV_PROJECT_ENVIRONMENT": session.virtualenv.location}
    if os.environ.get("CI", None) and sys.platform == "win32":
        env["SKBUILD_CMAKE_ARGS"] = "-T ClangCL"

    if shutil.which("cmake") is None and shutil.which("cmake3") is None:
        session.install("cmake")
    if shutil.which("ninja") is None:
        session.install("ninja")

    # install build and test dependencies on top of the existing environment
    session.run(
        "uv",
        "sync",
        "--inexact",
        "--only-group",
        "build",
        "--only-group",
        "test",
        *install_args,
        env=env,
    )
    if extra_command:
        session.run(*extra_command, env=env)
    if "--cov" in session.posargs:
        # try to use the lighter-weight `sys.monitoring` coverage core
        env["COVERAGE_CORE"] = "sysmon"
    session.run(
        "uv",
        "run",
        "--no-dev",  # do not auto-install dev dependencies
        "--no-build-isolation-package",
        "mqt-ddsim",  # build the project without isolation
        *install_args,
        "pytest",
        *run_args,
        *session.posargs,
        "--cov-config=pyproject.toml",
        env=env,
    )


@nox.session(python=PYTHON_ALL_VERSIONS, reuse_venv=True, default=True)
def tests(session: nox.Session) -> None:
    """Run the test suite."""
    _run_tests(session)


@nox.session(python=PYTHON_ALL_VERSIONS, reuse_venv=True, venv_backend="uv", default=True)
def minimums(session: nox.Session) -> None:
    """Test the minimum versions of dependencies."""
    with preserve_lockfile():
        _run_tests(
            session,
            install_args=["--resolution=lowest-direct"],
            run_args=["-Wdefault"],
        )
        env = {"UV_PROJECT_ENVIRONMENT": session.virtualenv.location}
        session.run("uv", "tree", "--frozen", env=env)


@nox.session(reuse_venv=True, venv_backend="uv", python=PYTHON_ALL_VERSIONS)
def qiskit(session: nox.Session) -> None:
    """Tests against the latest version of Qiskit."""
    with preserve_lockfile():
        _run_tests(
            session,
            extra_command=["uv", "pip", "install", "qiskit[qasm3-import] @ git+https://github.com/Qiskit/qiskit.git"],
        )
        env = {"UV_PROJECT_ENVIRONMENT": session.virtualenv.location}
        session.run("uv", "pip", "show", "qiskit", env=env)


@nox.session(reuse_venv=True)
def docs(session: nox.Session) -> None:
    """Build the docs. Use "--non-interactive" to avoid serving. Pass "-b linkcheck" to check links."""
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", dest="builder", default="html", help="Build target (default: html)")
    args, posargs = parser.parse_known_args(session.posargs)

    serve = args.builder == "html" and session.interactive
    if serve:
        session.install("sphinx-autobuild")

    env = {"UV_PROJECT_ENVIRONMENT": session.virtualenv.location}
    # install build and docs dependencies on top of the existing environment
    session.run(
        "uv",
        "sync",
        "--inexact",
        "--only-group",
        "build",
        "--only-group",
        "docs",
        env=env,
    )

    # build the C++ API docs using doxygen
    with session.chdir("docs"):
        if shutil.which("doxygen") is None:
            session.error("doxygen is required to build the C++ API docs")

        Path("_build/doxygen").mkdir(parents=True, exist_ok=True)
        session.run("doxygen", "Doxyfile", external=True)
        Path("api/cpp").mkdir(parents=True, exist_ok=True)
        session.run(
            "breathe-apidoc",
            "-o",
            "api/cpp",
            "-m",
            "-f",
            "-g",
            "file",
            "_build/doxygen/xml/",
            external=True,
        )

    shared_args = (
        "-n",  # nitpicky mode
        "-T",  # full tracebacks
        f"-b={args.builder}",
        "docs",
        f"docs/_build/{args.builder}",
        *posargs,
    )

    session.run(
        "uv",
        "run",
        "--no-dev",  # do not auto-install dev dependencies
        "--no-build-isolation-package",
        "mqt-ddsim",  # build the project without isolation
        "sphinx-autobuild" if serve else "sphinx-build",
        *shared_args,
        env=env,
    )


@nox.session(reuse_venv=True, venv_backend="uv")
def stubs(session: nox.Session) -> None:
    """Generate type stubs for Python bindings using nanobind."""
    env = {"UV_PROJECT_ENVIRONMENT": session.virtualenv.location}
    session.run(
        "uv",
        "sync",
        "--no-dev",
        "--group",
        "build",
        env=env,
    )

    package_root = Path(__file__).parent / "python" / "mqt" / "ddsim"
    pattern_file = Path(__file__).parent / "bindings" / "ddsim_patterns.txt"

    session.run(
        "python",
        "-m",
        "nanobind.stubgen",
        "--recursive",
        "--include-private",
        "--output-dir",
        package_root,
        "--pattern-file",
        pattern_file,
        "--module",
        "mqt.ddsim.pyddsim",
    )

    pyi_files = list(package_root.glob("**/*.pyi"))

    if not pyi_files:
        session.warn("No .pyi files found")
        return

    if shutil.which("prek") is None:
        session.install("prek")

    # Allow both 0 (no issues) and 1 as success codes for fixing up stubs
    success_codes = [0, 1]
    session.run("prek", "run", "license-tools", "--files", *pyi_files, external=True, success_codes=success_codes)
    session.run("prek", "run", "ruff-check", "--files", *pyi_files, external=True, success_codes=success_codes)
    session.run("prek", "run", "ruff-format", "--files", *pyi_files, external=True, success_codes=success_codes)

    # Run ruff-check again to ensure everything is clean
    session.run("prek", "run", "ruff-check", "--files", *pyi_files, external=True)


if __name__ == "__main__":
    nox.main()

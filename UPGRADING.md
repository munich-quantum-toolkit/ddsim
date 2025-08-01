# Upgrade Guide

This document describes breaking changes and how to upgrade. For a complete list of changes including minor and patch releases, please refer to the [changelog](CHANGELOG.md).

## [Unreleased]

This major release introduces several breaking changes, including the removal of deprecated features.
The following paragraphs describe the most important changes and how to adapt your code accordingly.
We intend to provide a more comprehensive migration guide for future releases.

The major change in this major release is the move to the MQT Core Python package.
This move allows us to make `qiskit` a fully optional dependency and entirely rely on the MQT Core IR for representing circuits.
Additionally, the `mqt-core` Python package now ships all its C++ libraries as shared libraries so that these need not be fetched or built as part of the build process.
This was tricky to achieve cross-platform, and you can find some more backstory in the corresponding PR [#336].
The problem was simplified by the latest `pybind11` release (`v3`) that greatly increased binary compatibility.
It is not necessary to build MQT Core from source, and a simple `uv sync` is enough to successfully run `pytest`.
We expect the MQT Core integration to mature over the next few releases.
If you encounter any issues, please let us know.

Support for the tensor network strategy in the path simulator has been removed.
If you still depend on that method, please use the last version of MQT DDSIM that supports them, which is `1.24.0`.

MQT Core itself dropped support for several parsers in `v3.0.0`, including the `.real`, `.qc`, `.tfc`, and `GRCS` parsers.
The `.real` parser lives on as part of the [MQT SyReC] project. All others have been removed without replacement.
Consequently, these input formats are no longer supported in MQT DDSIM.

MQT DDSIM has moved to the [munich-quantum-toolkit](https://github.com/munich-quantum-toolkit) GitHub organization under https://github.com/munich-quantum-toolkit/ddsim.
While most links should be automatically redirected, please update any links in your code to point to the new location.
All links in the documentation have been updated accordingly.

MQT DDSIM now requires CMake 3.24 or higher.
Most modern operating systems should have this version available in their package manager.
Alternatively, CMake can be conveniently installed from PyPI using the [`cmake`](https://pypi.org/project/cmake/) package.

MQT DDSIM now supports Qiskit 2.0.
As a result, the return values of the `Estimator` and `Sampler` have been changed to align with Qiskit's implementations.

To developers of MQT DDSIM, it is worth mentioning that all Python code (except tests) has been moved to the top-level `python` directory.
Furthermore, the C++ code for the Python bindings has been moved to the top-level `bindings` directory.

The `ConstructionMode`, `HybridMode`, and `PathSimulatorMode` enums are now exposed via `pybind11`'s new `py::native_enum`, which makes them compatible with Python's `enum.Enum` class (PEP 435).
As a result, the enums can no longer be initialized using a string.
Instead of `PathSimulatorMode("sequential")` or `"sequential"`, use `PathSimulatorMode.sequential`.

Finally, the minimum required C++ version has been raised from C++17 to C++20.
The default compilers of our test systems support all relevant features of the standard.

<!-- Version links -->

[unreleased]: https://github.com/munich-quantum-toolkit/qmap/compare/v1.24.0...HEAD

<!-- Other links -->

[#336]: https://github.com/munich-quantum-toolkit/ddsim/pull/336
[MQT SyReC]: https://github.com/cda-tum/mqt-syrec

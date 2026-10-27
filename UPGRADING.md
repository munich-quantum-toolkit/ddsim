# Upgrade Guide

This document describes breaking changes and how to upgrade. For a complete list
of changes including minor and patch releases, please refer to the
[changelog](CHANGELOG.md).

## [Unreleased]

### MQT DDSIM 3

MQT DDSIM uses MQT Core 4 for simulation and ships as a pure Python package. Use
Python 3.11 or newer and Qiskit `>=2.5,<3`. MQT Core supplies NumPy and the
native runtime dependencies.

| MQT DDSIM 2                                                           | MQT DDSIM 3                                                                                           |
| --------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| `qasm_simulator`, `QasmSimulatorBackend`                              | `sampling_simulator`, `SamplingSimulatorBackend`                                                      |
| Statevector and unitary Qiskit backends                               | `simulate(circuit)` and `build_functionality(circuit)` return NumPy arrays                            |
| `primitives.Sampler`                                                  | `backend.sampler()` returns Sampler V2                                                                |
| `primitives.Estimator`                                                | `primitives.DDEstimator` for exact DD expectations, or `backend.estimator()` for sampled Estimator V2 |
| Implicit measure-all                                                  | Add `circuit.measure_all()` or explicit measurements before sampling                                  |
| `seed=-1`                                                             | Use `None`; fixed seeds must be positive signed 32-bit integers                                       |
| `parameter_binds`                                                     | Qiskit `assign_parameters`, or backend `parameter_values` in `tuple(circuit.parameters)` order        |
| `mqt-ddsim-simple` and other executables                              | Use Python `sample`, `simulate`, and `build_functionality`                                            |
| Native simulator classes, DD access, node statistics, visualization   | Use MQT Core's program and DD APIs                                                                    |
| C++ headers/library and `pyddsim`                                     | Use MQT Core's C++ program, QCO, DD, or QDMI APIs                                                     |
| HSF, path, noise, approximation, and specialized algorithm simulators | Remain available in the MQT DDSIM v2 release series                                                   |

```python
from qiskit import QuantumCircuit
from mqt.ddsim import DDSIMProvider, simulate, build_functionality
from mqt.ddsim.primitives import DDEstimator

circuit = QuantumCircuit(2)
circuit.h(0)
circuit.cx(0, 1)
state = simulate(circuit)
unitary = build_functionality(circuit)
exact = DDEstimator().run([(circuit, "ZZ")]).result()

backend = DDSIMProvider().get_backend("sampling_simulator")
sampled = backend.estimator(default_precision=0.05).run([(circuit, "ZZ")]).result()
circuit.measure_all()
counts = backend.run(circuit, shots=1024).result().get_counts()
shots = backend.sampler().run([circuit], shots=1024).result()
```

The backend accepts operations in its advertised target. Transpile composite
user-defined gates with `transpile(circuit, backend)` before backend or Sampler
submission. Direct simulation functions use Core's Qiskit importer. Bind
OpenQASM inputs with Core's program `bind_parameters` method.

Statevectors describe an uncollapsed state; eligible terminal measurements are
deferred. Reset and measurement-dependent feedback are rejected. Full-unitary
construction rejects measurements and resets. Exact `DDEstimator` reports zero
sampling uncertainty and does not use a seed. Positive precision does not switch
it to sampling; use `backend.estimator()` for finite-shot estimation and dynamic
preparation.

Sampling is asynchronous through QDMI. Use `max_workers=1` for serial execution
or `0` for automatic concurrency. Fixed seeds reproduce samples only with the
same program, build, execution path, shot count, and worker configuration.
Legacy `result(timeout=...)` is not supported; native timed waits are available
through Core's QDMI job API.

QIR input requires an explicit `ProgramFormat`: text for `QIR_BASE_STRING` or
`QIR_ADAPTIVE_STRING`, bitcode bytes for their `MODULE` counterparts. Profile
declarations do not validate arbitrary LLVM. QIR full-unitary extraction is
unsupported.

#### Development dependency and release gate

The development branch pins an exact MQT Core Git revision. Installing that
revision builds Core with a C++20 toolchain and LLVM/MLIR 23.1; Core can
download a compatible SDK on supported hosts. Before publishing MQT DDSIM 3,
replace the Git pin with a suitable released Core version, validate installed
wheels, and run the supported CI matrix on the release candidate.

## [2.7.0]

This release updates the minimum required `mqt-core` version to 3.11.0. Rebuild
C++ libraries and Python extensions against the MQT Core 3.11 shared-library
ABI.

## [2.6.0]

This release updates the minimum required `mqt-core` version to 3.10.0.

### Ideal circuit layouts

`CircuitSimulator` now honors `initialLayout` and `outputPermutation`. Returned
state vectors and observables use the output order; explicit measurements keep
their classical bit destinations. Remove manual relabeling that compensated for
previously ignored layouts. This change applies to the ideal circuit simulator.

### Qiskit 2.1 minimum

The minimum Qiskit version increases from **1.1.0 to 2.1.0**, dropping support
for all Qiskit 1.x releases and Qiskit 2.0. Upgrade Qiskit to 2.1.0 or newer.

### Python 3.11 and Stable ABI wheels

MQT DDSIM now requires Python 3.11 or newer. Upgrade the Python environment
before installing this release.

MQT DDSIM now publishes one `cp311-abi3` wheel for GIL-enabled CPython 3.11 and
newer. Free-threaded support starts with CPython 3.15 in a separate
`cp315-abi3t` wheel. MQT DDSIM no longer publishes free-threaded CPython 3.13 or
3.14 wheels.

This release updates `nanobind` to 3.0.1, which changes the `nanobind` ABI.

### macOS support

MQT DDSIM no longer supports x86 macOS. Use Apple silicon with macOS 13.3 or
newer. The new deployment target enables `std::format` in libc++.

### CMake 3.28 minimum

MQT DDSIM now requires CMake 3.28 or newer. Upgrade CMake before building this
release.

### CMake presets on Windows

All CMake presets now use Ninja. On Windows, remove `-windows` from preset names
when configuring, building, and testing:

| Previous preset   | Replacement |
| ----------------- | ----------- |
| `debug-windows`   | `debug`     |
| `release-windows` | `release`   |

Install Ninja and run CMake from a Visual Studio developer shell for the target
architecture. Use a new build directory if an existing directory uses the Visual
Studio generator.

## [2.5.0]

This release updates the minimum required `mqt-core` version to 3.9.0 and
`nanobind` to 2.15.0.

### Vendored density-matrix decision-diagram support

Support for density-matrix decision diagrams will be removed from `mqt-core` in
version 4.0.0. Because the noise-aware simulators depend on it, MQT DDSIM now
vendors this functionality in the new `dd::ddsim` namespace. The simulator
interfaces are unchanged, except that `DeterministicNoiseSimulator::rootEdge` is
now a `dd::ddsim::DensityMatrixDD` instead of a `dd::DensityMatrixDD`.

## [2.4.0]

This release updates the minimum required `mqt-core` version to 3.7.0 as well as
the `nanobind` version to 2.13.0.

### CMake presets

[CMake presets] have been added to provide a standardized and reproducible way
to configure builds across different platforms. These presets are also used in
our CI.

On Unix systems, the `debug`, `release`, and `coverage` presets can be used to
configure, build, and test MQT DDSIM.

```console
cmake --preset release
cmake --build --preset release
ctest --preset release
```

Additionally, the `lint` preset can be used to configure and build MQT DDSIM in
preparation for a `clang-tidy` run.

If you are on Windows, use the `debug-windows` and `release-windows` presets.

## [2.3.0]

This release updates the minimum required `mqt-core` version to 3.6.0 as well as
the `nanobind` version to 2.12.0.

## [2.2.0]

### Python wheels

This release contains two changes to the distributed wheels.

First, we have removed all wheels for Python 3.13t. Free-threading Python was
introduced as an experimental feature in Python 3.13. It became stable in Python
3.14.

Second, for Python 3.12+, we are now providing Stable ABI wheels instead of
separate version-specific wheels. This was enabled by migrating our Python
bindings from `pybind11` to `nanobind`.

Both of these changes were made in the interest of conserving PyPI space and
reducing CI/CD build times. The full list of wheels now reads:

- 3.10
- 3.11
- 3.12+ Stable ABI
- 3.14t

## [2.1.0]

### End of support for Python 3.9

Starting with this release, MQT DDSIM no longer supports Python 3.9. This is in
line with the scheduled end of life of the version. As a result, MQT DDSIM is no
longer tested under Python 3.9 and no longer ships Python 3.9 wheels.

## [2.0.0]

This major release introduces several breaking changes, including the removal of
deprecated features. The following paragraphs describe the most important
changes and how to adapt your code accordingly. We intend to provide a more
comprehensive migration guide for future releases.

The major change in this major release is the move to the MQT Core Python
package. This move allows us to make `qiskit` a fully optional dependency and
entirely rely on the MQT Core IR for representing circuits. Additionally, the
`mqt-core` Python package now ships all its C++ libraries as shared libraries so
that these need not be fetched or built as part of the build process. This was
tricky to achieve cross-platform, and you can find some more backstory in the
corresponding PR [#336]. The problem was simplified by the latest `pybind11`
release (`v3`) that greatly increased binary compatibility. It is not necessary
to build MQT Core from source, and a simple `uv sync` is enough to successfully
run `pytest`. We expect the MQT Core integration to mature over the next few
releases. If you encounter any issues, please let us know.

Support for the tensor network strategy in the path simulator has been removed.
If you still depend on that method, please use the last version of MQT DDSIM
that supports them, which is `1.24.0`.

MQT Core itself dropped support for several parsers in `v3.0.0`, including the
`.real`, `.qc`, `.tfc`, and `GRCS` parsers. The `.real` parser lives on as part
of the [MQT SyReC] project. All others have been removed without replacement.
Consequently, these input formats are no longer supported in MQT DDSIM.

MQT DDSIM has moved to the
[munich-quantum-toolkit](https://github.com/munich-quantum-toolkit) GitHub
organization under <https://github.com/munich-quantum-toolkit/ddsim>. While most
links should be automatically redirected, please update any links in your code
to point to the new location. All links in the documentation have been updated
accordingly.

MQT DDSIM now requires CMake 3.24 or higher. Most modern operating systems
should have this version available in their package manager. Alternatively,
CMake can be conveniently installed from PyPI using the
[`cmake`](https://pypi.org/project/cmake/) package.

MQT DDSIM now supports Qiskit 2.0. As a result, the return values of the
`Estimator` and `Sampler` have been changed to align with Qiskit's
implementations.

To developers of MQT DDSIM, it is worth mentioning that all Python code (except
tests) has been moved to the top-level `python` directory. Furthermore, the C++
code for the Python bindings has been moved to the top-level `bindings`
directory.

Furthermore, many Python modules and classes have been renamed. In particular,

- `HybridCircuitSimulator` has been renamed to `HybridSimulator`,
- `HybridMode` has been renamed to `HybridSimulatorMode`,
- `PathCircuitSimulator` has been renamed to `PathSimulator`, and
- `ConstructionMode` has been renamed to `UnitarySimulatorMode`.
- Some of the Qiskit backends have been renammed. For the new names, see
  `DDSIMProvider.get_backend()`.

The `UnitarySimulatorMode`, `HybridSimulatorMode`, and `PathSimulatorMode` enums
are now exposed via `pybind11`'s new `py::native_enum`, which makes them
compatible with Python's `enum.Enum` class (PEP 435). As a result, the enums can
no longer be initialized using a string. Instead of
`PathSimulatorMode("sequential")` or `"sequential"`, use
`PathSimulatorMode.sequential`.

Finally, the minimum required C++ version has been raised from C++17 to C++20.
The default compilers of our test systems support all relevant features of the
standard.

<!-- Version links -->

[unreleased]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.7.0...HEAD
[2.7.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.6.0...v2.7.0
[2.6.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.5.0...v2.6.0
[2.5.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.4.0...v2.5.0
[2.4.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.3.0...v2.4.0
[2.3.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.2.0...v2.3.0
[2.2.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.1.0...v2.2.0
[2.1.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v2.0.0...v2.1.0
[2.0.0]: https://github.com/munich-quantum-toolkit/ddsim/compare/v1.24.0...v2.0.0

<!-- Other links -->

[#336]: https://github.com/munich-quantum-toolkit/ddsim/pull/336
[MQT SyReC]: https://github.com/munich-quantum-toolkit/syrec
[CMake presets]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html

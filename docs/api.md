# Python API

## Simulation functions

```{autofunction} mqt.ddsim.simulate
```

```{autofunction} mqt.ddsim.sample
```

`mqt.ddsim.build_functionality` directly exposes
[MQT Core's full-unitary construction](https://mqt.readthedocs.io/projects/core/en/latest/).
It accepts a bound Qiskit circuit, OpenQASM source/path, or supported Core
program and returns a complex NumPy matrix.

## Qiskit integration

```{autoclass} mqt.ddsim.SamplingSimulatorBackend
```

The backend inherits execution, target validation, result reconstruction, and V2
primitive factories from
[MQT Core's QDMI integration](https://mqt.readthedocs.io/projects/core/en/latest/qdmi/qdmi_backend.html).
See {doc}`primitives` for `sampler()`, `estimator()`, and the directly
re-exported `mqt.ddsim.primitives.DDEstimator`.

```{autoclass} mqt.ddsim.DDSIMProvider
:members: get_backend, backends
```

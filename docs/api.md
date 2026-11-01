# Python API

## Simulation functions

MQT DDSIM directly exposes these MQT Core functions:

| Function                                                                                                                                            | Result                                |
| --------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------- |
| [`simulate(program)`](https://mqt.readthedocs.io/projects/core/en/latest/api/mqt/core/mlir/index.html#mqt.core.mlir.simulate)                       | Uncollapsed complex NumPy statevector |
| [`sample(program, shots=1024, seed=0)`](https://mqt.readthedocs.io/projects/core/en/latest/api/mqt/core/mlir/index.html#mqt.core.mlir.sample)       | Measurement counts                    |
| [`build_functionality(program)`](https://mqt.readthedocs.io/projects/core/en/latest/api/mqt/core/mlir/index.html#mqt.core.mlir.build_functionality) | Full complex NumPy unitary matrix     |

Use bound Qiskit circuits, OpenQASM source/paths, or supported Core programs.
Sampling and statevector simulation also accept compiled QIR and LLVM
text/bitcode. See {doc}`simulators` for examples and input limits.

## Qiskit integration

```{eval-rst}
.. autoclass:: mqt.ddsim.SamplingSimulatorBackend
```

The backend inherits execution, target validation, result reconstruction, and V2
primitive factories from
[MQT Core's QDMI integration](https://mqt.readthedocs.io/projects/core/en/latest/qdmi/qdmi_backend.html).
See {doc}`primitives` for `sampler()`, `estimator()`, and the directly
re-exported `mqt.ddsim.primitives.DDEstimator`.

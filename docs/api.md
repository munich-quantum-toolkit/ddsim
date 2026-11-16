# Python API

## Simulation functions

Choose a simulation function by the result you need:

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
.. autoclass:: mqt.core.plugins.qiskit.backend.DDSIMBackend
```

`SamplingSimulatorBackend` is an alias for MQT Core's `DDSIMBackend`. Use
`compile()` to prepare a program for the device and `run()` to submit it. See
{doc}`primitives` for `sampler()`, `estimator()`, and
`mqt.ddsim.primitives.DDEstimator`.

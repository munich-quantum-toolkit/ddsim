# mqt.ddsim

```{py:module} mqt.ddsim
```

MQT DDSIM re-exports the following MQT Core functions and classes. Follow the
links for signatures, supported inputs, and options.

| Entry point                                                                                       | Purpose                                          |
| ------------------------------------------------------------------------------------------------- | ------------------------------------------------ |
| {external+core:py:func}`~mqt.core.mlir.sample`                                                    | Measurement counts                               |
| {external+core:py:func}`~mqt.core.mlir.simulate`                                                  | Uncollapsed statevector as a NumPy array         |
| {external+core:py:func}`~mqt.core.mlir.build_functionality`                                       | Full unitary matrix as a NumPy array             |
| {external+core:py:class}`SamplingSimulatorBackend <mqt.core.plugins.qiskit.backend.DDSIMBackend>` | Qiskit backend for counts and shot memory        |
| {external+core:py:class}`primitives.DDEstimator <mqt.core.plugins.qiskit.estimator.DDEstimator>`  | Exact expectation values using decision diagrams |

For a `SamplingSimulatorBackend` instance called `backend`, use
`backend.sampler()` for Sampler V2 and `backend.estimator()` for sampled
Estimator V2.

See the {doc}`simulation guide </simulation>` for execution modes and supported
inputs, and {doc}`Qiskit primitives </primitives>` for sampling and estimation
examples.

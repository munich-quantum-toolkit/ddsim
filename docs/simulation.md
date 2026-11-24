# Simulation

MQT DDSIM uses MQT Core's decision-diagram engine for all execution modes.
Choose the result you need:

| Result                   | Entry point                                | Program requirements                                                                                   |
| ------------------------ | ------------------------------------------ | ------------------------------------------------------------------------------------------------------ |
| Measurement counts       | `sample` or `SamplingSimulatorBackend.run` | Explicit measurements with supported classical feedback and resets                                     |
| Per-register shot arrays | `backend.sampler()`                        | Measured Qiskit circuits in Sampler V2 PUBs                                                            |
| Uncollapsed statevector  | `simulate`                                 | Bound program without resets or measurement-dependent feedback. Terminal measurements may be deferred. |
| Full unitary             | `build_functionality`                      | Bound unitary program without measurements or resets                                                   |
| Exact expectations       | `DDEstimator`                              | Qiskit state preparation supported by MQT Core's statevector simulation                                |
| Sampled expectations     | `backend.estimator()`                      | Qiskit circuits supported by the backend, including dynamic preparation                                |

Qiskit circuits and OpenQASM 2/3 source strings or paths support sampling,
statevectors, and unitaries subject to the requirements above. Bind circuit
parameters with `assign_parameters`. To bind OpenQASM inputs, create an MQT Core
program from the source and call its `bind_parameters` method. The Qiskit
backend accepts operations in its advertised target. Prepare composite gates
with `backend.compile(circuit)` or `transpile(circuit, backend)`.

Dense statevectors require space proportional to $2^n$, and full matrices to
$4^n$. Sampling and exact DD estimation avoid dense result extraction. A compact
DD can therefore simulate programs whose dense arrays would exhaust memory.

```{toctree}
:maxdepth: 1

simulation/sampling
simulation/statevectors_and_unitaries
simulation/qir
```

:::{note}
Hybrid Schrödinger–Feynman, path, noise, and specialized algorithm simulators
remain available in the
[MQT DDSIM v2 release series](https://github.com/munich-quantum-toolkit/ddsim/releases/tag/v2.7.0).
:::

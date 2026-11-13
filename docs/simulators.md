# Simulation

MQT DDSIM uses MQT Core's decision-diagram engine for all execution modes.
Choose the result you need:

| Result                   | Entry point                                | Program requirements                                                                             |
| ------------------------ | ------------------------------------------ | ------------------------------------------------------------------------------------------------ |
| Measurement counts       | `sample` or `SamplingSimulatorBackend.run` | Explicit measurements; supported classical feedback and resets                                   |
| Per-register shot arrays | `backend.sampler()`                        | Measured Qiskit circuits; Sampler V2 PUBs                                                        |
| Uncollapsed statevector  | `simulate`                                 | Bound program; terminal measurements may be deferred; no reset or measurement-dependent feedback |
| Full unitary             | `build_functionality`                      | Bound unitary program; no measurements or resets                                                 |
| Exact expectations       | `DDEstimator`                              | Qiskit state preparation supported by Core's statevector simulation                              |
| Sampled expectations     | `backend.estimator()`                      | Qiskit circuits supported by the backend, including dynamic preparation                          |

Qiskit circuits and OpenQASM 2/3 source strings or paths support sampling,
statevectors, and unitaries subject to the requirements above. Bind circuit
parameters with `assign_parameters`; bind OpenQASM inputs through MQT Core's
program `bind_parameters` method. The Qiskit backend accepts circuits in its
advertised target; compile composite gates with `transpile(circuit, backend)`.

Dense statevectors require space proportional to $2^n$, and full matrices to
$4^n$. Sampling and exact DD estimation avoid dense result extraction. A compact
DD can therefore simulate programs whose dense arrays would exhaust memory.

```{toctree}
:maxdepth: 1

simulators/sampling
simulators/statevectors_and_unitaries
simulators/qir
```

:::{note}
Hybrid Schrödinger–Feynman, path, noise, and specialized algorithm simulators
remain available in the
[MQT DDSIM v2 release series](https://github.com/munich-quantum-toolkit/ddsim/releases/tag/v2.7.0).
:::

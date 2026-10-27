---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Qiskit V2 primitives

MQT DDSIM offers two estimation methods and a Sampler V2 through MQT Core's
Qiskit integration. All accept Qiskit PUBs and follow their parameter ordering
and broadcasting rules.

| Primitive            | Construction          | Result                                                                        |
| -------------------- | --------------------- | ----------------------------------------------------------------------------- |
| Sampler V2           | `backend.sampler()`   | One shot array per classical register                                         |
| Exact DD Estimator   | `DDEstimator()`       | Expectations from the DD state, without repeated sampling or dense extraction |
| Sampled Estimator V2 | `backend.estimator()` | Finite-shot estimates, including dynamic state preparation                    |

## Sampling parameter sweeps

Parameters follow `tuple(circuit.parameters)` order. PUB-level shots override
the run-level setting, which overrides `default_shots`. Register separation and
PUB broadcast dimensions are preserved in the result.

```{code-cell} python
import numpy as np
from qiskit import QuantumCircuit
from qiskit.circuit import Parameter
from mqt.ddsim import SamplingSimulatorBackend

angle = Parameter("angle")
circuit = QuantumCircuit(1)
circuit.ry(angle, 0)
measured = circuit.copy()
measured.measure_all()
angles = np.linspace(0, np.pi, 9).reshape(-1, 1)
backend = SamplingSimulatorBackend()
sampler = backend.sampler(default_shots=512, seed_simulator=71)
result = sampler.run([(measured, angles)]).result()[0]
result.data.meas.get_counts(4)
```

## Exact and sampled expectations

`DDEstimator` evaluates Pauli observables on a coherent DD state. It supports
the same state preparations as Core's statevector simulation. It avoids dense
statevector extraction, but the DD can still grow exponentially. Its `stds=0`
means zero sampling uncertainty, not zero quantum variance or infinite numerical
accuracy. Precision does not switch it to sampling.

Sampled Estimator V2 requires positive precision and supports a broader class of
circuits, including resets and classical feedback. It groups commuting
observables and obtains expectations from measurement shots.

```{code-cell} python
from mqt.ddsim.primitives import DDEstimator

exact = DDEstimator().run([(circuit, "Z", angles)]).result()[0]
sampled = backend.estimator(default_precision=0.05, seed_simulator=71).run(
    [(circuit, "Z", angles)]
).result()[0]
```

```{code-cell} python
import matplotlib.pyplot as plt

fig, ax = plt.subplots(figsize=(7, 3.5), layout="constrained")
ax.plot(angles[:, 0], exact.data.evs, label="Exact DD")
ax.errorbar(angles[:, 0], sampled.data.evs, yerr=sampled.data.stds,
            fmt="o", capsize=3, label="Sampled ± standard error")
ax.set(xlabel="Rotation angle (radians)", ylabel="Expectation of Z")
ax.legend()
plt.show()
```

Inspect metadata for the selected method, precision, and sampling information:

```{code-cell} python
exact.metadata, sampled.metadata
```

A reset is a valid sampled preparation, but it does not define the coherent
state required by the exact estimator:

```{code-cell} python
dynamic = QuantumCircuit(1)
dynamic.h(0)
dynamic.reset(0)
backend.estimator(default_precision=0.1).run([(dynamic, "Z")]).result()[0].data.evs
```

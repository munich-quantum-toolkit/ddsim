---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Qiskit V2 primitives

MQT DDSIM exposes Qiskit's native backend Sampler and Core's native exact DD
Estimator. PUB binding columns follow `circuit.parameters`, not gate insertion
order. Leading parameter axes and observables follow Qiskit broadcasting.

## Sampler

```{code-cell} python
from qiskit import QuantumCircuit
from mqt.ddsim.primitives import Sampler

circuit = QuantumCircuit(2)
circuit.x(0)
circuit.measure_all()
result = Sampler().run([(circuit,)], shots=16).result()
assert result[0].data.meas.get_counts() == {"01": 16}
```

Each classical register has a separate `BitArray`, preserving shot correlations
and PUB shape. PUB shots override run shots, which override the constructor
default. No classical registers means an empty `DataBin`; add measurements
explicitly. Native metadata reports shots, circuit metadata and version 2.

## Exact and sampled estimation

```{code-cell} python
from qiskit.quantum_info import SparsePauliOp
from mqt.ddsim import DDSIMProvider
from mqt.ddsim.primitives import Estimator

preparation = QuantumCircuit(2)
preparation.h(0)
preparation.cx(0, 1)
observable = SparsePauliOp("ZZ")
exact = Estimator().run([(preparation, observable)]).result()
assert abs(float(exact[0].data.evs) - 1) < 1e-12
backend = DDSIMProvider().get_backend("qasm_simulator")
job = backend.estimator().run([(preparation, observable)], precision=0.05)
sampled = job.result()
assert abs(float(sampled[0].data.evs) - 1) < 1e-12
```

`Estimator` exposes Core's `DDEstimator`. It prepares one pure-state DD per
binding point and evaluates all observables for that point in a native batch. It
needs the complete state represented as a DD, not a dense amplitude array.
Finite real Pauli sums are supported; invalid dimensions, coefficients and
numerical overflow fail. Qiskit rejects zero or cancelled observable sums during
normal PUB coercion; Core's program-level API accepts zero observables directly.
Eligible terminal measurements are deferred; feedback and reset are rejected
instead of returning one trajectory's expectation.

The compatibility `seed` constructor argument and property are retained but have
no effect on exact execution. Precision defaults to zero. Every finite
nonnegative precision requests exact numerical evaluation; positive precision
does not add noise or switch methods. PUB precision overrides run precision and
then the default. `evs` and `stds` have the PUB broadcast shape. `stds=0` means
zero sampling uncertainty, not zero quantum variance or numerical error.
Metadata contains `method="exact"`, target precision and circuit metadata,
without shots.

`backend.estimator()` remains Qiskit's sampled `BackendEstimatorV2`. It requires
positive precision and uses `ceil(1 / precision**2)` shots per measurement
circuit, with Pauli grouping. Its `stds` are statistical error estimates. This
separate path retains finite-shot semantics and supported dynamic preparation.
Sampling still represents quantum states and does not guarantee lower memory
costs for a large DD. Methods never switch silently after a resource error.
Arbitrary QIR is not a Qiskit PUB input.

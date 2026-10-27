---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Sampling

Use the Qiskit backend to retrieve counts and individual shots. Classical
registers retain their names and order; separate registers are separated by
spaces in Qiskit count strings.

```{code-cell} python
from qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister
from mqt.ddsim import SamplingSimulatorBackend

q = QuantumRegister(2, "q")
readout = ClassicalRegister(1, "readout")
feedback = ClassicalRegister(1, "feedback")
circuit = QuantumCircuit(q, readout, feedback)
circuit.h(0)
circuit.measure(0, readout)
with circuit.if_test((readout[0], True)):
    circuit.x(1)
circuit.measure(1, feedback)
circuit.draw("mpl")
```

```{code-cell} python
from qiskit.visualization import plot_histogram

backend = SamplingSimulatorBackend()
result = backend.run(
    circuit, shots=1024, memory=True, seed_simulator=71, max_workers=1
).result()
plot_histogram(result.get_counts(), title="Measurement-controlled preparation")
```

```{code-cell} python
result.get_memory()[:10]
```

`seed_simulator` accepts `None` for randomness or a positive signed 32-bit
integer. `max_workers=0` selects automatic concurrency; `1` runs serially.
Reproducibility requires the same program, build, execution path, shot count,
and worker configuration. Seeds do not promise identical samples across
simulation paths or releases.

For parameterized circuits, `run(..., parameter_values=[values])` binds values
in `tuple(circuit.parameters)` order. For broadcast parameter sweeps, use
{doc}`Sampler V2 <../primitives>`.

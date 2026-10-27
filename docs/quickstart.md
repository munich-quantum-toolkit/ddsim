---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Quickstart

Create a Bell pair, measure it, and inspect the correlations. Qiskit circuits
use qubit zero as the least-significant bit in result strings.

```{code-cell} python
from qiskit import QuantumCircuit
from qiskit.visualization import plot_histogram
from mqt.ddsim import DDSIMProvider

circuit = QuantumCircuit(3)
circuit.h(0)
circuit.cx(0, 2)
circuit.measure_all()
circuit.draw("mpl")
```

```{code-cell} python
backend = DDSIMProvider().get_backend("sampling_simulator")
counts = backend.run(circuit, shots=1024, seed_simulator=17).result().get_counts()
plot_histogram(counts, title="Bell pair on qubits 0 and 2")
```

Measurements are explicit: only the circuit's classical outputs appear in the
result. For per-register shot arrays and parameter sweeps, use
{doc}`Sampler V2 <primitives>`.

OpenQASM source strings and file paths work with the direct sampling function:

```{code-cell} python
from mqt.ddsim import sample

source = '''OPENQASM 3;
include "stdgates.inc";
qubit[2] q;
bit[2] c;
x q[0];
c = measure q;'''
sample(source, shots=16)
```

Continue with {doc}`simulators/sampling` for classical feedback and memory,
{doc}`simulators/statevectors_and_unitaries` for dense results, or
{doc}`primitives` for expectation values.

---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Quickstart

Use the familiar provider for Qiskit counts:

```{code-cell} python
from qiskit import QuantumCircuit
from mqt.ddsim import DDSIMProvider, simulate, build_functionality

circuit = QuantumCircuit(3)
circuit.h(0)
circuit.cx(0, 2)
backend = DDSIMProvider().get_backend("qasm_simulator")
counts = backend.run(circuit, shots=1024, seed_simulator=17).result().get_counts()
assert set(counts) == {"000", "101"}
assert sum(counts.values()) == 1024
state = simulate(circuit)
unitary = build_functionality(circuit)
assert state.shape == (8,)
assert unitary.shape == (8, 8)
```

The sampling backend measures all qubits on a copy when a legacy circuit has no
classical bits. V2 Sampler reports only registers in its original PUB; add
measurements explicitly for Sampler results.

For OpenQASM, pass a file path or a source string beginning with its version:

```{code-cell} python
from mqt.ddsim import sample

source = '''OPENQASM 3.0;
include "stdgates.inc";
qubit[2] q;
bit[2] c;
x q[0];
c = measure q;'''
assert sample(source, shots=16) == {"01": 16}
```

QIR requires an explicit profile and matching payload type:

```python
from pathlib import Path
from mqt.core.qdmi import ProgramFormat
from mqt.ddsim import sample

counts = sample(
    Path("program.bc").read_bytes(),
    program_format=ProgramFormat.QIR_ADAPTIVE_MODULE,
    shots=1024,
)
```

Use `QIR_BASE_STRING` or `QIR_ADAPTIVE_STRING` for LLVM text strings and the
corresponding `MODULE` enum for bitcode bytes. QIR receives a fully bound
program; arbitrary LLVM entry arguments are not DDSIM parameter bindings.

An invalid or missing QIR format raises `ValueError`. A payload type that does
not match its explicit format raises `TypeError`. Detected execution failures
raise `RuntimeError`; a format declaration does not validate arbitrary LLVM.

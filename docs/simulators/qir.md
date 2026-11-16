# QIR execution

Pass LLVM text as `str`, bitcode as `bytes`, or a compiled `QIRProgram` to
`sample` or `simulate`. The program metadata specifies the QIR profile and
version. The program must be fully bound and use operations supported by MQT
Core's QIR runtime.

```python
from pathlib import Path
from mqt.ddsim import sample

counts = sample(
    Path("program.bc").read_bytes(),
    shots=1024,
)
```

Base programs contain a unitary prefix followed by terminal measurements.
Adaptive programs can use supported classical feedback, resets, and resource
management. Output records determine the sampling bit order. `simulate` extracts
an uncollapsed state only when the program permits terminal measurements to be
deferred; it rejects reset and measurement-dependent feedback. Statevector
indices follow QIR runtime qubit order, which can differ from source-circuit
order.

Direct sampling uses `seed=0` for randomness. Fixed QIR seeds must be positive
signed 32-bit integers.

QIR entry points must take no arguments. Missing or unsupported profiles raise
`ValueError`; execution failures raise `RuntimeError`. QIR supports sampling and
statevector extraction, but not full-unitary construction.

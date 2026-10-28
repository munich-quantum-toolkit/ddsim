# QIR execution

Pass LLVM text as `str`, bitcode as `bytes`, or a compiled `QIRProgram` to
`sample` or `simulate`. Core reads the profile and version from the program's
metadata. The program must be fully bound and use operations supported by MQT
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
management. Sampling returns QDMI output-record ordering. `simulate` extracts an
uncollapsed state only when the program permits terminal measurements to be
deferred; it rejects reset and measurement-dependent feedback. Statevector
indices follow QIR runtime qubit order, which can differ from source-circuit
order.

Direct sampling uses `seed=0` for randomness. Fixed QIR seeds must be positive
signed 32-bit integers.

Profile metadata does not validate arbitrary LLVM programs. Missing or
unsupported profiles raise `ValueError`; detected execution failures raise
`RuntimeError`. Arbitrary LLVM entry arguments are not parameter bindings.
Full-unitary extraction from executable QIR is unsupported.

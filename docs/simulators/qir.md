# QIR execution

Pass LLVM text as `str` or bitcode as `bytes`, together with its explicit QDMI
profile. The program must be fully bound and use operations supported by MQT
Core's QIR runtime. Compiled `QIRProgram` objects can be passed directly to
`sample` and `simulate`, without an explicit format.

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

| Profile  | LLVM text             | Bitcode               |
| -------- | --------------------- | --------------------- |
| Base     | `QIR_BASE_STRING`     | `QIR_BASE_MODULE`     |
| Adaptive | `QIR_ADAPTIVE_STRING` | `QIR_ADAPTIVE_MODULE` |

Base programs contain a unitary prefix followed by terminal measurements.
Adaptive programs can use supported classical feedback, resets, and resource
management. Sampling returns QDMI output-record ordering. `simulate` extracts an
uncollapsed state only when the program permits terminal measurements to be
deferred; it rejects reset and measurement-dependent feedback. Statevector
indices follow QIR runtime qubit order, which can differ from source-circuit
order.

Direct sampling uses `seed=0` for randomness. Fixed QIR seeds must be positive
signed 32-bit integers.

Profile declarations are caller guarantees, not conformance validation of
arbitrary LLVM programs. Payload/profile mismatches raise `TypeError`;
unsupported format selections raise `ValueError`; detected execution failures
raise `RuntimeError`. Arbitrary LLVM entry arguments are not parameter bindings.
Full-unitary extraction from executable QIR is unsupported.

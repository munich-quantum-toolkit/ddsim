# Simulation modes

MQT DDSIM uses MQT Core for import, lowering and execution.

| Mode        | Result                                | Input restrictions                                                                         |
| ----------- | ------------------------------------- | ------------------------------------------------------------------------------------------ |
| Sampling    | Counts or per-shot Qiskit memory      | Supported measurements, resets and classical feedback                                      |
| Statevector | Uncollapsed complex amplitudes        | Coherent preparation; eligible terminal measurements deferred; reset and feedback rejected |
| Unitary     | Full matrix on all input basis states | No measurement, reset or runtime allocation; QIR unsupported                               |

Qubit zero is the least-significant basis bit. Statevectors and unitaries retain
global phase and idle wires. Dense output grows exponentially; counts and exact
expectations never require dense extraction. One sampled trajectory is not an
uncollapsed state or an exact ensemble mean.

OpenQASM 2.0, 3.0 and 3.1 use
[Core's supported subset](https://mqt.readthedocs.io/projects/core/en/stable/mlir/OpenQASM.html).
Bind Qiskit parameters before simulation. OpenQASM parameters can be bound with
Core's program `bind_parameters` API before passing the program to DDSIM.
Unsupported syntax and unbound parameters raise Core diagnostics.

QIR accepts explicit Base/Adaptive declarations for LLVM text or bitcode. Valid
Base programs contain a unitary prefix followed by terminal measurements; reset
requires Adaptive. Profile declarations are not full conformance validation.
Callers must provide the supported QIR entry-point ABI and profile; MQT DDSIM
does not validate arbitrary LLVM programs. Detected execution or
result-extraction failures propagate as errors. Dynamic state extraction follows
Core's allocation order and retains one wire per allocation lifetime.

```{toctree}
:maxdepth: 1

simulators/CircuitSimulator
simulators/UnitarySimulator
```

Removed HSF, path, noise and specialized APIs are covered in the
{doc}`migration guide <UPGRADING>`.

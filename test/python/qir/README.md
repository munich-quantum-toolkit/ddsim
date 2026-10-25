# QIR execution fixtures

These original OpenQASM programs and emitted QIR were captured with MQT Core
`1d384a5d23aa1f02b26fd5684dab586ef26167fe` (QIR 2.1). The `.ll` files are
readable originals of the submitted programs. The `.bc` files are the same
modules in LLVM bitcode.

To reproduce emission, use
`mqt.core.mlir.compile_program(source, output=OutputFormat.QIR_BASE)` for
`asymmetric` and `phase`, and `OutputFormat.QIR_ADAPTIVE` for `feedback`. Export
with `to_llvm_ir(copy=True)` and `to_bitcode()`.

The independent expected results follow directly from each source:

- `asymmetric`: q0 and q1 are one, q2 is zero. Recorded bits reverse to `011`.
  The compiler removes idle q3, so the QIR resource declaration has three qubits
  and the state has amplitude one at index 3.
- `phase`: q1 is one and q0 is in a Hadamard superposition. Controlled adjoint S
  multiplies the q0=1 branch by -i; global phase multiplies both amplitudes by
  exp(0.37i). The two nonzero amplitudes are at indices 2 and 3.
- `feedback`: the first measurement controls q1, and resetting q0 makes its
  final measurement zero. Only `000` and `011` occur, each with probability one
  half.

The assertions compare output order and phase directly. No invalid Base-profile
reset program is part of the supported-input tests.

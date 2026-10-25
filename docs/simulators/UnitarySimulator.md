# Unitary simulation

`mqt.ddsim.build_functionality(circuit)` constructs a full complex128 matrix
through Core. The retained Qiskit entry point is
`DDSIMProvider().get_backend("unitary_simulator")`; call
`run(circuit).result().get_unitary()` for its matrix.

The result has `matrix[row, column] = <row|U|column>` with qubit zero as the
least-significant bit. Global phase and declared idle wires are retained.
Measurements, resets and unsupported allocation are errors, including terminal
measurements. QIR unitary simulation is unsupported: a zero-state execution does
not determine the operator on other inputs.

The backend runs synchronously and returns a completed Qiskit job. Shots,
memory, seeds and recursive/sequential construction modes are not accepted. The
native `UnitarySimulator` and its DD methods are removed. Use Core's
`qco::buildFunctionality` for compact C++ matrix DDs.

```console
mqt-ddsim-simple --simulate_file circuit.qasm --mode unitary
```

Dense matrices require memory proportional to `4**num_qubits`; request them only
when needed. Exact Pauli estimation does not build a dense operator.

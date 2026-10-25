# Sampling and statevector simulation

`mqt.ddsim.sample` returns counts through Core's DD sampler. Qiskit backend
`qasm_simulator` uses Core's bundled QDMI device and returns standard Qiskit
results with optional per-shot memory. It supports `parameter_values`, one value
set per circuit, and `seed_simulator`. `parameter_binds` is rejected. Sampling
seeds are `None`, `-1`, or integers from 1 through `INT_MAX`. `max_workers=0`
selects automatic concurrency; `max_workers=1` runs serially. Positive worker
limits through `INT_MAX` are accepted. Seed reproducibility requires the same
program, bindings, versions, execution path, shots, worker configuration, and
capture settings.

For composite gates outside the backend target, call
`qiskit.transpile(circuit, backend)` before sampling. The backend uses its
advertised native operations and does not insert an automatic transpilation
pass.

`mqt.ddsim.simulate` and `statevector_simulator` return an uncollapsed state,
with eligible terminal measurements deferred. Reset, classical feedback and
later use of measured wires are rejected. The state backend supplies no counts;
call the sampling backend separately. Qiskit's `Result.get_counts` may itself
convert a stored statevector to probabilities; these are not sampled counts. The
synchronous state `run` finishes before returning a completed job, so it cannot
cancel an in-progress calculation. Shots, memory and stochastic seeds are
invalid state backend options.

Seeds are `None`, legacy `-1`, or positive integers through native `INT_MAX`.
Zero is rejected. Reproducibility requires the same program, values, versions,
entry point, algorithm, shots, workers and capture settings. It is not promised
across engines, versions or histogram/memory modes.

## Command line

```console
mqt-ddsim-simple --simulate_file circuit.qasm --shots 1024 --seed 17 --pm
mqt-ddsim-simple --simulate_file circuit.qasm --pv
mqt-ddsim-simple --simulate_file program.bc --input-format qir-adaptive --mode sample
```

JSON output has `schema_version: 1` and `mode`. Counts have a `counts` map and
`shots`. Dense results have `shape`, `data` and
`complex_encoding: "[real, imaginary]"`. `--ps` adds input metadata and elapsed
time, not internal DD statistics. `--pv` selects state output and conflicts with
sampling options. Invalid arguments exit with status 2; execution failures exit
with status 1 and a JSON error on standard error.

The native `CircuitSimulator` and raw-DD methods are removed. Advanced compact
state users use Core's DD/QCO API with explicit ownership; see
{doc}`the migration guide <../UPGRADING>`.

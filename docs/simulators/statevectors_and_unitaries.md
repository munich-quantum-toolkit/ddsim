---
file_format: mystnb
kernelspec:
  name: python3
mystnb:
  number_source_lines: true
---

# Statevectors and unitaries

`simulate` returns amplitudes from the all-zero input. `build_functionality`
returns the full unitary, with input basis states indexing columns and output
basis states indexing rows. Both use qubit zero as the least-significant bit and
preserve the program's global phase.

```{code-cell} python
import numpy as np
from qiskit import QuantumCircuit
from qiskit.quantum_info import Statevector
from mqt.ddsim import simulate, build_functionality

circuit = QuantumCircuit(2, global_phase=0.37)
circuit.h(0)
circuit.cx(0, 1)
circuit.sdg(1)
circuit.draw("mpl")
```

```{code-cell} python
state = simulate(circuit)
Statevector(state).draw("latex", decimals=4)
```

The phase is part of the returned amplitudes, even though computational-basis
probabilities alone cannot reveal it.

```{code-cell} python
import matplotlib.pyplot as plt

unitary = build_functionality(circuit)
fig, axes = plt.subplots(1, 2, figsize=(9, 3.5), layout="constrained")
for ax, values, title, cmap, bounds in [
    (axes[0], np.abs(unitary), "Magnitude", "viridis", (0, 1)),
    (axes[1], np.angle(unitary), "Phase (radians)", "twilight", (-np.pi, np.pi)),
]:
    image = ax.imshow(values, cmap=cmap, vmin=bounds[0], vmax=bounds[1])
    ax.set(title=title, xlabel="Input basis state", ylabel="Output basis state")
    ax.set_xticks(range(4), ["00", "01", "10", "11"])
    ax.set_yticks(range(4), ["00", "01", "10", "11"])
    fig.colorbar(image, ax=ax)
plt.show()
```

Eligible terminal measurements are deferred by statevector simulation: the
result is the uncollapsed state, not a randomly selected trajectory. Reset and
measurement-dependent feedback are rejected. Full-unitary construction rejects
all measurements and resets. These functions return NumPy arrays; for large
compact DDs, use MQT Core's DD APIs without materializing dense results.

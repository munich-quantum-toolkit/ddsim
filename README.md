[![PyPI](https://img.shields.io/pypi/v/mqt.ddsim?logo=pypi&style=flat-square)](https://pypi.org/project/mqt.ddsim/)
![OS](https://img.shields.io/badge/os-linux%20%7C%20macos%20%7C%20windows-blue?style=flat-square)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![CI](https://img.shields.io/github/actions/workflow/status/munich-quantum-toolkit/ddsim/ci.yml?branch=main&style=flat-square&logo=github&label=ci)](https://github.com/munich-quantum-toolkit/ddsim/actions/workflows/ci.yml)
[![CD](https://img.shields.io/github/actions/workflow/status/munich-quantum-toolkit/ddsim/cd.yml?style=flat-square&logo=github&label=cd)](https://github.com/munich-quantum-toolkit/ddsim/actions/workflows/cd.yml)
[![Documentation](https://img.shields.io/readthedocs/ddsim?logo=readthedocs&style=flat-square)](https://mqt.readthedocs.io/projects/ddsim)
[![codecov](https://img.shields.io/codecov/c/github/munich-quantum-toolkit/ddsim?style=flat-square&logo=codecov)](https://codecov.io/gh/munich-quantum-toolkit/ddsim)

<p align="center">
  <a href="https://mqt.readthedocs.io">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-banner-dark.svg" width="90%">
      <img src="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-banner-light.svg" width="90%" alt="MQT Banner">
    </picture>
  </a>
</p>

# MQT DDSIM - A quantum circuit simulator based on decision diagrams written in C++

A tool for classical quantum circuit simulation developed as part of the [_Munich Quantum Toolkit (MQT)_](https://mqt.readthedocs.io).
It builds upon [MQT Core](https://github.com/munich-quantum-toolkit/core), which forms the backbone of the MQT.

<p align="center">
  <a href="https://mqt.readthedocs.io/projects/ddsim">
  <img width=30% src="https://img.shields.io/badge/documentation-blue?style=for-the-badge&logo=read%20the%20docs" alt="Documentation" />
  </a>
</p>

## Key Features

- Decision-diagram–based circuit simulation: [Circuit Simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/CircuitSimulator.html)—strong (statevector) and weak (sampling), incl. mid‑circuit measurements and resets; Qiskit backends [qasm_simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/CircuitSimulator.html#usage-as-a-qiskit-backend) • [statevector_simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/CircuitSimulator.html#usage-as-a-qiskit-backend). [Quickstart](https://mqt.readthedocs.io/projects/ddsim/en/latest/quickstart.html) • [API](https://mqt.readthedocs.io/projects/ddsim/en/latest/api/mqt/ddsim/index.html)
- Unitary simulation: [Unitary Simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/UnitarySimulator.html) with an optional [alternative recursive construction](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/UnitarySimulator.html#alternative-construction-sequence) for improved intermediate compactness.
- Hybrid Schrödinger–Feynman: [Hybrid simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/HybridSchrodingerFeynman.html) trading memory for runtime with DD and amplitude modes plus multithreading; also available as a statevector backend.
- Simulation Path Framework: [Path-based simulation](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/SimulationPathFramework.html) with strategies [sequential](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/SimulationPathFramework.html#simulating-a-simple-circuit), [pairwise_recursive](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/SimulationPathFramework.html#configuration), [bracket](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/SimulationPathFramework.html#configuration), and [alternating](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/SimulationPathFramework.html#configuration).
- Noise-aware simulation: [Stochastic and deterministic noise](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/NoiseAwareSimulator.html) (amplitude damping, depolarization, phase flip; density-matrix mode) for global decoherence and gate errors.
- Qiskit-native API: Provider backends and Primitives—[Sampler](https://mqt.readthedocs.io/projects/ddsim/en/latest/primitives.html#sampler) • [Estimator](https://mqt.readthedocs.io/projects/ddsim/en/latest/primitives.html#estimator)—for algorithm-friendly workflows. [API](https://mqt.readthedocs.io/projects/ddsim/en/latest/api/mqt/ddsim/index.html)
- Decision-diagram visualization: inspect states/unitaries via Graphviz export; see [Circuit Simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/CircuitSimulator.html) and [Unitary Simulator](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/UnitarySimulator.html).
- Standalone CLI: fast C++ executables with JSON output; e.g., [ddsim_simple](https://mqt.readthedocs.io/projects/ddsim/en/latest/simulators/CircuitSimulator.html#usage-as-standalone-c-executable).
- Efficient and portable: C++20 core with DD engines; prebuilt wheels for Linux/macOS/Windows via [PyPI](https://pypi.org/project/mqt.ddsim/).

If you have any questions, feel free to create a [discussion](https://github.com/munich-quantum-toolkit/ddsim/discussions) or an [issue](https://github.com/munich-quantum-toolkit/ddsim/issues) on [GitHub](https://github.com/munich-quantum-toolkit/ddsim).

## Contributors and Supporters

The _[Munich Quantum Toolkit (MQT)](https://mqt.readthedocs.io)_ is developed by the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the [Technical University of Munich](https://www.tum.de/) and supported by the [Munich Quantum Software Company (MQSC)](https://munichquantum.software).
Among others, it is part of the [Munich Quantum Software Stack (MQSS)](https://www.munich-quantum-valley.de/research/research-areas/mqss) ecosystem, which is being developed as part of the [Munich Quantum Valley (MQV)](https://www.munich-quantum-valley.de) initiative.

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-logo-banner-dark.svg" width="90%">
    <img src="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-logo-banner-light.svg" width="90%" alt="MQT Partner Logos">
  </picture>
</p>

Thank you to all the contributors who have helped make MQT DDSIM a reality!

<p align="center">
  <a href="https://github.com/munich-quantum-toolkit/ddsim/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=munich-quantum-toolkit/ddsim" alt="Contributors to munich-quantum-toolkit/ddsim" />
  </a>
</p>

The MQT will remain free, open-source, and permissively licensed—now and in the future.
We are firmly committed to keeping it open and actively maintained for the quantum computing community.

To support this endeavor, please consider:

- Starring and sharing our repositories: https://github.com/munich-quantum-toolkit
- Contributing code, documentation, tests, or examples via issues and pull requests
- Citing the MQT in your publications (see [Cite This](#cite-this))
- Citing our research in your publications (see [References](https://mqt.readthedocs.io/projects/ddsim/en/latest/references.html))
- Using the MQT in research and teaching, and sharing feedback and use cases
- Sponsoring us on GitHub: https://github.com/sponsors/munich-quantum-toolkit

<p align="center">
  <a href="https://github.com/sponsors/munich-quantum-toolkit">
  <img width=20% src="https://img.shields.io/badge/Sponsor-white?style=for-the-badge&logo=githubsponsors&labelColor=black&color=blue" alt="Sponsor the MQT" />
  </a>
</p>

## Getting Started

MQT DDSIM bundled with the provider and backends for Qiskit is available via [PyPI](https://pypi.org/project/mqt.ddsim/) for Linux, macOS, and Windows and supports Python 3.9 to 3.14.

```console
(venv) $ pip install mqt.ddsim
```

The following code gives an example on the usage:

```python3
from qiskit import QuantumCircuit
from mqt import ddsim

circ = QuantumCircuit(3)
circ.h(0)
circ.cx(0, 1)
circ.cx(0, 2)

print(circ.draw(fold=-1))

backend = ddsim.DDSIMProvider().get_backend("qasm_simulator")

job = backend.run(circ, shots=10000)
counts = job.result().get_counts(circ)
print(counts)
```

**Detailed documentation on all available methods, options, and input formats is available at [ReadTheDocs](https://mqt.readthedocs.io/projects/ddsim).**

## System Requirements and Building

The implementation is compatible with any C++20 compiler and a minimum CMake version of 3.24.
Please refer to the [documentation](https://mqt.readthedocs.io/projects/ddsim) on how to build the project.

Building (and running) is continuously tested under Linux, macOS, and Windows using the [latest available system versions for GitHub Actions](https://github.com/actions/virtual-environments).

## Cite This

Please cite the work that best fits your use case.

### MQT DDSIM (the tool)

When citing the software itself or results produced with it, cite the original DD simulation paper:

```bibtex
@article{zulehner2019advanced,
	title        = {Advanced Simulation of Quantum Computations},
	author       = {Zulehner, Alwin and Wille, Robert},
	year         = 2019,
	journal      = {tcad},
	volume       = 38,
	number       = 5,
	pages        = {848--859},
	doi          = {10.1109/TCAD.2018.2834427}
}
```

### The Munich Quantum Toolkit (the project)

When discussing the overall MQT project or its ecosystem, cite the MQT Handbook:

```bibtex
@inproceedings{mqt,
	title        = {The {{MQT}} Handbook: {{A}} Summary of Design Automation Tools and Software for Quantum Computing},
	shorttitle   = {{The MQT Handbook}},
	author       = {Wille, Robert and Berent, Lucas and Forster, Tobias and Kunasaikaran, Jagatheesan and Mato, Kevin and Peham, Tom and Quetschlich, Nils and Rovara, Damian and Sander, Aaron and Schmid, Ludwig and Schoenberger, Daniel and Stade, Yannick and Burgholzer, Lukas},
	year         = 2024,
	booktitle    = {IEEE International Conference on Quantum Software (QSW)},
	doi          = {10.1109/QSW62656.2024.00013},
	eprint       = {2405.17543},
	eprinttype   = {arxiv},
	addendum     = {A live version of this document is available at \url{https://mqt.readthedocs.io}}
}
```

### Peer-Reviewed Research

When citing the underlying methods and research, please reference the most relevant peer-reviewed publications from the list below:

[[1]](https://www.cda.cit.tum.de/files/eda/2018_tcad_advanced_simulation_quantum_computation.pdf)
A. Zulehner and R. Wille. Advanced Simulation of Quantum Computations.
_IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems (TCAD)_, 2019.

[[2]](https://www.cda.cit.tum.de/files/eda/2020_dac_weak_simulation_quantum_computation.pdf)
S. Hillmich, I. L. Markov, and R. Wille. Just Like the Real Thing: Fast Weak Simulation of Quantum Computation.
In _Design Automation Conference (DAC)_, 2020.

[[3]](https://www.cda.cit.tum.de/files/eda/2021_date_approximations_dd_baed_quantum_circuit_simulation.pdf)
S. Hillmich, R. Kueng, I. L. Markov, and R. Wille. As Accurate as Needed, as Efficient as Possible: Approximations in DD-based Quantum Circuit Simulation.
In _Design, Automation and Test in Europe (DATE)_, 2021.

[[4]](https://www.cda.cit.tum.de/files/eda/2021_qce_hybrid_schrodinger_feynman_simulation_with_decision_diagrams.pdf)
L. Burgholzer, H. Bauer, and R. Wille. Hybrid Schrödinger–Feynman Simulation of Quantum Circuits with Decision Diagrams.
In _IEEE International Conference on Quantum Computing and Engineering (QCE)_, 2021.

[[5]](https://www.cda.cit.tum.de/files/eda/2022_date_exploiting_arbitrary_paths_simulation_quantum_circuits_decision_diagrams.pdf)
L. Burgholzer, A. Ploier, and R. Wille. Exploiting Arbitrary Paths for the Simulation of Quantum Circuits with Decision Diagrams.
In _Design, Automation and Test in Europe (DATE)_, 2022.

[[6]](https://www.cda.cit.tum.de/files/eda/2022_tcad_noise-aware_quantum_circuit_simulation_with_decision_diagrams.pdf)
T. Grurl, J. Fuß, and R. Wille. Noise-aware Quantum Circuit Simulation with Decision Diagrams.
_IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems (TCAD)_, 2022.

---

## Acknowledgements

The Munich Quantum Toolkit has been supported by the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation program (grant agreement No. 101001318), the Bavarian State Ministry for Science and Arts through the Distinguished Professorship Program, as well as the Munich Quantum Valley, which is supported by the Bavarian state government with funds from the Hightech Agenda Bayern Plus.

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-funding-footer-dark.svg" width="90%">
    <img src="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-funding-footer-light.svg" width="90%" alt="MQT Funding Footer">
  </picture>
</p>

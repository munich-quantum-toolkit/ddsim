# MQT DDSIM

A quantum circuit simulator based on decision diagrams powered by MQT Core

Explore quantum programs with compact decision diagrams: sample measurements,
inspect statevectors and unitaries, or evaluate observables. MQT DDSIM connects
Qiskit, OpenQASM, and QIR to MQT Core's native simulation engine as part of the
{doc}`Munich Quantum Toolkit <mqt:index>`.

MQT DDSIM supports Python 3.11 or newer and Qiskit `>=2.5,<3`.

::::{grid} 1 2 2 2
:gutter: 3

:::{grid-item-card}

{doc}`Start simulating <quickstart>`. Build a circuit, sample its measurements,
and plot the results.
:::

:::{grid-item-card}

{doc}`Choose an execution mode <simulators>`. Work with counts, amplitudes,
matrices, or QIR programs.
:::

:::{grid-item-card}

{doc}`Estimate observables <primitives>`. Use exact DD estimation or sampled
Estimator V2 for dynamic circuits.
:::

:::{grid-item-card}

{doc}`Python API <api>`. Find simulation functions and the Qiskit backend.
:::

::::

```{toctree}
:hidden:

self
```

```{toctree}
:hidden:
:maxdepth: 1
:caption: User Guide

installation
quickstart
simulators
primitives
references
CHANGELOG
UPGRADING
```

```{toctree}
:hidden:
:maxdepth: 2
:titlesonly:
:caption: Developers
:glob:

contributing
ai_usage
tooling
support
```

```{toctree}
:hidden:
:caption: Python API Reference
:maxdepth: 1

api
```

## Contributors and Supporters

The _[Munich Quantum Toolkit (MQT)](https://mqt.readthedocs.io)_ is developed by
the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the
[Technical University of Munich](https://www.tum.de/) and supported by
[MQSC](https://mq.sc). Among others, it is part of the
[Munich Quantum Software Stack (MQSS)](https://www.munich-quantum-valley.de/research/research-areas/mqss)
ecosystem, which is being developed as part of the
[Munich Quantum Valley (MQV)](https://www.munich-quantum-valley.de) initiative.

<div style="margin-top: 0.5em">
<div class="only-light" align="center">
  <img src="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-logo-banner-light.svg" width="90%" alt="MQT Banner">
</div>
<div class="only-dark" align="center">
  <img src="https://raw.githubusercontent.com/munich-quantum-toolkit/.github/refs/heads/main/docs/_static/mqt-logo-banner-dark.svg" width="90%" alt="MQT Banner">
</div>
</div>

Thank you to all the contributors who have helped make MQT DDSIM a reality!

<p align="center">
<a href="https://github.com/munich-quantum-toolkit/ddsim/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=munich-quantum-toolkit/ddsim" alt="Contributors to munich-quantum-toolkit/ddsim" />
</a>
</p>

The MQT will remain free, open-source, and permissively licensed—now and in the
future. We are firmly committed to keeping it open and actively maintained for
the quantum computing community.

To support this endeavor, please consider:

- Starring and sharing our repositories:
  <https://github.com/munich-quantum-toolkit>
- Contributing code, documentation, tests, or examples via issues and pull
  requests
- Citing the MQT in your publications (see {doc}`References <references>`)
- Using the MQT in research and teaching, and sharing feedback and use cases
- Sponsoring us on GitHub:
  <https://github.com/sponsors/munich-quantum-toolkit>

<p align="center">
<iframe src="https://github.com/sponsors/munich-quantum-toolkit/button" title="Sponsor munich-quantum-toolkit" height="32" width="114" style="border: 0; border-radius: 6px;"></iframe>
</p>

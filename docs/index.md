# MQT DDSIM - A quantum circuit simulator based on Decision Diagrams

```{raw} latex
\begin{abstract}
```

MQT DDSIM is an open-source C++20 and Python library for classical quantum circuit simulation developed as part of the _{doc}`Munich Quantum Toolkit (MQT) <mqt:index>`_.

This documentation provides a comprehensive guide to the MQT DDSIM library, including {doc}`installation instructions <installation>`, a {doc}`quickstart guide <quickstart>`, and detailed {doc}`API documentation <api/mqt/ddsim/index>`.
The source code of MQT DDSIM is publicly available on GitHub at [munich-quantum-toolkit/ddsim](https://github.com/munich-quantum-toolkit/ddsim), while pre-built binaries are available via [PyPI](https://pypi.org/project/mqt.ddsim/) for all major operating systems and all modern Python versions.
MQT DDSIM is fully compatible with Qiskit 1.0 and above.

````{only} latex
```{note}
A live version of this document is available at [mqt.readthedocs.io/projects/ddsim](https://mqt.readthedocs.io/projects/ddsim).
```
````

```{raw} latex
\end{abstract}

\sphinxtableofcontents
```

```{toctree}
:hidden:

self
```

```{toctree}
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

````{only} not latex
```{toctree}
:maxdepth: 2
:titlesonly:
:caption: Developers
:glob:

contributing
support
development_guide
```
````

```{toctree}
:caption: Python API Reference
:maxdepth: 1

api/mqt/ddsim/index
```

```{toctree}
:glob:
:caption: C++ API Reference
:maxdepth: 1

api/cpp/filelist
```

```{only} html
## Contributors and Supporters

The _[Munich Quantum Toolkit (MQT)](https://mqt.readthedocs.io)_ is developed by the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the [Technical University of Munich](https://www.tum.de/) and supported by the [Munich Quantum Software Company (MQSC)](https://munichquantum.software).
Among others, it is part of the [Munich Quantum Software Stack (MQSS)](https://www.munich-quantum-valley.de/research/research-areas/mqss) ecosystem, which is being developed as part of the [Munich Quantum Valley (MQV)](https://www.munich-quantum-valley.de) initiative.

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

The MQT will remain free, open-source, and permissively licensed—now and in the future.
We are firmly committed to keeping it open and actively maintained for the quantum computing community.

To support this endeavor, please consider:

- Starring and sharing our repositories: [https://github.com/munich-quantum-toolkit](https://github.com/munich-quantum-toolkit)
- Contributing code, documentation, tests, or examples via issues and pull requests
- Citing the MQT in your publications (see {doc}`References <references>`)
- Using the MQT in research and teaching, and sharing feedback and use cases
- Sponsoring us on GitHub: [https://github.com/sponsors/munich-quantum-toolkit](https://github.com/sponsors/munich-quantum-toolkit)

<p align="center">
<iframe src="https://github.com/sponsors/munich-quantum-toolkit/button" title="Sponsor munich-quantum-toolkit" height="32" width="114" style="border: 0; border-radius: 6px;"></iframe>
</p>
```

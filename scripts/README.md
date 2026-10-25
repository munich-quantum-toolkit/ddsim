# MQT DDSIM installed-wheel check

`check_wheel_runtime.py` checks an installed wheel from outside the source
checkout. It exercises device discovery, Qiskit backends and V2 primitives,
OpenQASM, QIR text and bitcode, exact state and unitary results, and the CLI.
Run it with the Python interpreter from a clean environment containing the built
MQT DDSIM wheel and its dependencies:

```console
cd /tmp
/path/to/environment/bin/python /path/to/ddsim/scripts/check_wheel_runtime.py
```

The previous approximation, HSF and specialized Shor benchmark scripts depend on
removed MQT DDSIM 2.x simulator implementations. To reproduce those published
experiments, use the matching historical repository revision and dependency
versions.

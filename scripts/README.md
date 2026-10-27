# MQT DDSIM installed-wheel check

`check_wheel_runtime.py` checks an installed wheel from outside the source
checkout. It exercises device discovery, the Qiskit backend and V2 primitives,
OpenQASM, QIR bitcode, and exact state and unitary results. Run it with the
Python interpreter from a clean environment containing the built MQT DDSIM wheel
and its dependencies:

```console
cd /tmp
/path/to/environment/bin/python /path/to/ddsim/scripts/check_wheel_runtime.py
```

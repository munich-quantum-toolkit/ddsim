OPENQASM 3.0;
include "stdgates.inc";
qubit[2] q;
h q[0];
bit flag = measure q[0];
if (flag) { x q[1]; }
reset q[0];
bit answer = measure q[1];
bit clean = measure q[0];

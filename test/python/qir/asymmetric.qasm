OPENQASM 3.0;
include "stdgates.inc";
qubit[4] q;
x q[0];
x q[1];
bit[2] a;
a[0] = measure q[0];
a[1] = measure q[1];
bit b = measure q[2];

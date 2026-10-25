OPENQASM 3.0;
include "stdgates.inc";
qubit[2] q;
h q[0];
x q[1];
inv @ ctrl @ s q[1], q[0];
gphase(0.37);

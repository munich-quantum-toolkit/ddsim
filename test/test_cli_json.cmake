# Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
# Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

set(qasm_file "${CMAKE_CURRENT_BINARY_DIR}/cli-json.qasm")
file(
  WRITE "${qasm_file}"
  [=[OPENQASM 2.0;
include "qelib1.inc";
qreg q[1];
h q[0];
s q[0];
]=])

execute_process(
  COMMAND "${DDSIM_CLI}" --simulate_file "${qasm_file}" --pv
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "CLI failed (${result}): ${error}")
endif()

string(JSON vector_type TYPE "${output}" state_vector)
string(JSON vector_length LENGTH "${output}" state_vector)
if(NOT vector_type STREQUAL "ARRAY" OR NOT vector_length EQUAL 2)
  message(FATAL_ERROR "Expected a state vector with two entries: ${output}")
endif()

foreach(entry RANGE 0 1)
  string(JSON entry_type TYPE "${output}" state_vector ${entry})
  string(JSON entry_length LENGTH "${output}" state_vector ${entry})
  if(NOT entry_type STREQUAL "ARRAY" OR NOT entry_length EQUAL 2)
    message(FATAL_ERROR "Expected [real, imaginary] at entry ${entry}: ${output}")
  endif()

  foreach(component RANGE 0 1)
    string(
      JSON
      component_type
      TYPE
      "${output}"
      state_vector
      ${entry}
      ${component})
    string(
      JSON
      value
      GET
      "${output}"
      state_vector
      ${entry}
      ${component})
    if(entry EQUAL component)
      # The real part of |0> and imaginary part of |1> are 1/sqrt(2).
      set(lower 0.7071067811855475)
      set(upper 0.7071067811875475)
    else()
      set(lower -0.000000000001)
      set(upper 0.000000000001)
    endif()
    if(NOT component_type STREQUAL "NUMBER"
       OR value LESS lower
       OR value GREATER upper)
      message(FATAL_ERROR "Unexpected component ${entry},${component}: ${value}")
    endif()
  endforeach()
endforeach()

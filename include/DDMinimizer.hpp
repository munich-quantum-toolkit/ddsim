/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"

namespace ddsim {
/// Select a qubit order for ideal state-vector simulation with
/// CircuitSimulator.
class DDMinimizer {
public:
  /// Change only the circuit's input layout to the control-dependency order.
  ///
  /// Operations, registers, and the output permutation are preserved. This
  /// assumes the permutation-invariant all-zero input and a simulator that
  /// tracks layouts, such as CircuitSimulator. It is not supported by the
  /// noise, hybrid, or path simulators.
  static void optimizeInputPermutation(qc::QuantumComputation& circuit);

  /// Return a physical-qubit-to-DD-level mapping with targets below controls.
  ///
  /// Standard controlled gates contribute dependencies, including gates in
  /// compound operations. Controlled-Z gates and classically controlled
  /// operations are ignored. Ties preserve the existing logical qubit order.
  /// Cycles, empty circuits, and circuits with ancillary or garbage qubits
  /// retain their input layout. The heuristic does not guarantee a smaller DD.
  [[nodiscard]] static qc::Permutation
  createGateBasedPermutation(const qc::QuantumComputation& circuit);
};
} // namespace ddsim

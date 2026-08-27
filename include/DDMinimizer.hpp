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

#include "ir/Definitions.hpp"
#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"

#include <cstddef>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace qc {
/**
 * @brief Heuristically reorder circuit qubits for decision-diagram simulation.
 */
class DDMinimizer {
public:
  /**
   * @brief Optimize the input permutation of a quantum computation.
   * @details Canonicalizes any existing input permutation, computes a new
   * permutation from controlled-gate patterns, and applies it to the circuit.
   * The resulting input permutation is the identity and the output permutation
   * is adjusted accordingly. Since the input mapping is deliberately changed,
   * equivalence assumes the permutation-invariant all-zero input state used by
   * the simulators. Empty circuits and circuits with ancillary or garbage
   * qubits are left unchanged.
   * @param circuit The quantum computation to optimize.
   */
  static void optimizeInputPermutation(QuantumComputation& circuit);

  /**
   * @brief Compute a DD-friendly input permutation.
   * @details Collects the instruction indices of controlled-gate patterns and
   * derives a permutation from the most prominent complete pattern.
   * Controlled-Z operations are ignored. If no complete pattern is found, a
   * deterministic control-dependency ordering is used instead.
   * Circuits with ancillary or garbage qubits retain their existing input
   * layout.
   * @param circuit The quantum computation for which to compute a permutation.
   * @return The computed input permutation.
   * @pre Operations use dense qubit indices from zero to `getNqubits() - 1`.
   * @pre The input layout is the identity permutation. Use
   * `optimizeInputPermutation` for circuits with arbitrary input layouts.
   */
  [[nodiscard]] static Permutation
  createGateBasedPermutation(const QuantumComputation& circuit);

private:
  using InstructionIndex = std::optional<std::size_t>;
  using GatePattern = std::map<std::pair<Qubit, Qubit>, InstructionIndex>;

  /**
   * @brief Adjacent controlled-gate patterns.
   * @details The ladders x_c and c_x describe for four qubits the following
   * controlled gates (c: control qubit, x: target qubit):

   * c_x: c | 0  1  2
   *      x | 1  2  3

   * x_c: c | 1  2  3
   *      x | 0  1  2
   */
  GatePattern xCMap;
  GatePattern cXMap;

  /**
   * @brief Staircase controlled-gate patterns.
   * @details The ladders c_l, c_r, x_l, and x_r consist of several steps,
   * hence the vector of maps. They describe for four qubits the following
   * controlled gates (c: control qubit, x: target qubit):
   *
   * c_l_1: c | 0  0  0  and  c_l_2: c | 1  1  and  c_l_3: c | 2
   *        x | 1  2  3              x | 2  3              x | 3

   * c_r_1: c | 3  3  3  and  c_r_2: c | 2  2  and  c_r_3: c | 1
   *        x | 0  1  2              x | 0  1              x | 0

   * x_l_1: c | 1  2  3  and  x_l_2: c | 2  3  and  x_l_3: c | 3
   *        x | 0  0  0              x | 1  1              x | 2

   * x_r_1: c | 0  1  2  and  x_r_2: c | 0  1  and  x_r_3: c | 0
   *        x | 3  3  3              x | 2  2              x | 1
   */
  std::vector<GatePattern> cLMap;
  std::vector<GatePattern> cHMap;
  std::vector<GatePattern> xLMap;
  std::vector<GatePattern> xHMap;

  /**
   * @brief Initialize the controlled-gate pattern maps.
   * @param bits The number of qubits in the quantum computation.
   * @pre `bits >= 2`.
   */
  void initializeDataStructure(std::size_t bits);

  /**
   * @brief Find the final instruction index of a complete pattern.
   * @details A missing instruction index for any required gate marks the
   * pattern as incomplete, in which case this function returns `std::nullopt`.
   * @param pattern The pattern map to inspect.
   * @return The maximum instruction index, or `std::nullopt` for an incomplete
   * pattern.
   */
  static InstructionIndex getCompletePatternEnd(const GatePattern& pattern);

  // Functions to analyze the pattern of the controlled gates
  static bool isFullLadder(const std::vector<InstructionIndex>& vec);
  static std::size_t getStairCount(const std::vector<InstructionIndex>& vec);
  static std::size_t
  countPriorCompleteSteps(const std::vector<InstructionIndex>& steps,
                          const InstructionIndex& ladderEnd);

  // Functions to adjust the layout based on the pattern of the controlled
  // gates:

  /**
   * @brief Rotate a layout to the left.
   * @details `[0, 1, 2, 3]` rotated one step becomes `[1, 2, 3, 0]`.
   * @param layout The layout to rotate.
   * @param stairs The number of positions to rotate.
   * @return The rotated layout.
   */
  static std::vector<Qubit> rotateLeft(std::vector<Qubit> layout,
                                       std::size_t stairs);

  /**
   * @brief Rotate a layout to the right.
   * @details `[0, 1, 2, 3]` rotated one step becomes `[3, 0, 1, 2]`.
   * @param layout The layout to rotate.
   * @param stairs The number of positions to rotate.
   * @return The rotated layout.
   */
  static std::vector<Qubit> rotateRight(std::vector<Qubit> layout,
                                        std::size_t stairs);

  /**
   * @brief Create a permutation from control dependencies.
   * @details Orders each target before the qubits that control it. Controlled-Z
   * operations are ignored. If the dependency graph contains a cycle, the
   * existing input permutation is returned unchanged.
   * @param circuit The quantum computation to inspect.
   * @return The control-dependency-based input permutation.
   */
  static Permutation
  createControlBasedPermutation(const QuantumComputation& circuit);

}; // class DDMinimizer
} // namespace qc

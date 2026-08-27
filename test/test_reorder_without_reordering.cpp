/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DDMinimizer.hpp"
#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"

#include <gtest/gtest.h>

TEST(ReorderWithoutReorderingTest, reorderXc) {
  // control -> target
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 0}, {1, 1}, {2, 2}, {3, 3}}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(ReorderWithoutReorderingTest, reorderCx) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 3}, {1, 2}, {2, 1}, {3, 0}}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(ReorderWithoutReorderingTest, reorderXccl) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 1);
  circuit.cx(0, 2);
  circuit.cx(0, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 1}, {1, 2}, {2, 3}, {3, 0}}};

  EXPECT_EQ(expectedPerm, perm);
}

// failing
TEST(ReorderWithoutReorderingTest, reorderXcxh) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 3);
  circuit.cx(1, 3);
  circuit.cx(2, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 3}, {1, 0}, {2, 1}, {3, 2}}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(ReorderWithoutReorderingTest, reorderCxch) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.cx(1, 0);
  circuit.cx(2, 0);
  circuit.cx(3, 0);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 2}, {1, 1}, {2, 0}, {3, 3}}};

  EXPECT_EQ(expectedPerm, perm);
}

// failing
TEST(ReorderWithoutReorderingTest, reorderCxxl) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.cx(3, 0);
  circuit.cx(3, 1);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 0}, {1, 3}, {2, 2}, {3, 1}}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(ReorderWithoutReorderingTest, reorderInterlacedQubits) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 3);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{{0, 2}, {1, 3}, {2, 1}, {3, 0}}};

  EXPECT_EQ(expectedPerm, perm);
}

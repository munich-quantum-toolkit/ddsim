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

namespace {
void expectCircuitUnchanged(const qc::QuantumComputation& circuit,
                            const qc::QuantumComputation& original) {
  EXPECT_EQ(circuit.initialLayout, original.initialLayout);
  EXPECT_EQ(circuit.outputPermutation, original.outputPermutation);
  EXPECT_EQ(circuit.getAncillary(), original.getAncillary());
  EXPECT_EQ(circuit.getGarbage(), original.getGarbage());
  ASSERT_EQ(circuit.size(), original.size());

  auto originalOp = original.cbegin();
  for (const auto& op : circuit) {
    EXPECT_EQ(*op, **originalOp);
    ++originalOp;
  }
}
} // namespace

TEST(DDMinimizerTest, ReorderXc) {
  // control -> target
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderCx) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 3}, {1, 2}, {2, 1}, {3, 0}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderXcCl) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 1);
  circuit.cx(0, 2);
  circuit.cx(0, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderXcXh) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 3);
  circuit.cx(1, 3);
  circuit.cx(2, 3);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 3}, {1, 0}, {2, 1}, {3, 2}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderCxCh) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.cx(1, 0);
  circuit.cx(2, 0);
  circuit.cx(3, 0);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 2}, {1, 1}, {2, 0}, {3, 3}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderCxXl) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.cx(3, 0);
  circuit.cx(3, 1);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 0}, {1, 3}, {2, 2}, {3, 1}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, ReorderInterlacedQubits) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 3);
  circuit.cx(3, 2);

  const qc::Permutation perm =
      qc::DDMinimizer::createGateBasedPermutation(circuit);

  const qc::Permutation expectedPerm = {{0, 2}, {1, 3}, {2, 1}, {3, 0}};

  EXPECT_EQ(expectedPerm, perm);
}

TEST(DDMinimizerTest, HandlesFewerThanTwoQubits) {
  const qc::QuantumComputation emptyCircuit;
  EXPECT_TRUE(
      qc::DDMinimizer::createGateBasedPermutation(emptyCircuit).empty());

  const qc::QuantumComputation singleQubitCircuit(1);
  const qc::Permutation identity = {{0, 0}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(singleQubitCircuit),
            identity);
}

TEST(DDMinimizerTest, HandlesCompletePatternAtFirstInstruction) {
  qc::QuantumComputation ascendingCircuit(2);
  ascendingCircuit.cx(0, 1);
  const qc::Permutation reverse = {{0, 1}, {1, 0}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(ascendingCircuit),
            reverse);

  qc::QuantumComputation descendingCircuit(2);
  descendingCircuit.cx(1, 0);
  const qc::Permutation identity = {{0, 0}, {1, 1}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(descendingCircuit),
            identity);
}

TEST(DDMinimizerTest, ReorderXcClTwoStairs) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 1);
  circuit.cx(0, 2);
  circuit.cx(0, 3);
  circuit.cx(1, 2);
  circuit.cx(1, 3);

  const qc::Permutation expected = {{0, 2}, {1, 3}, {2, 0}, {3, 1}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), expected);
}

TEST(DDMinimizerTest, ReorderXcXhTwoStairs) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 3);
  circuit.cx(1, 3);
  circuit.cx(2, 3);
  circuit.cx(0, 2);
  circuit.cx(1, 2);

  const qc::Permutation expected = {{0, 2}, {1, 3}, {2, 0}, {3, 1}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), expected);
}

TEST(DDMinimizerTest, ReorderDescendingLadderWithPriority) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(0, 1);
  circuit.cx(0, 2);
  circuit.cx(0, 3);
  circuit.cx(1, 2);
  circuit.cx(2, 3);

  const qc::Permutation identity = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), identity);
}

TEST(DDMinimizerTest, AscendingPriorityDominatesOtherStairFamilies) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.cx(2, 0);
  circuit.cx(2, 1);
  circuit.cx(1, 0);
  circuit.cx(3, 2);
  circuit.cx(3, 0);

  const qc::Permutation reverse = {{0, 3}, {1, 2}, {2, 1}, {3, 0}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), reverse);
}

TEST(DDMinimizerTest, DescendingPriorityDominatesOtherStairFamilies) {
  qc::QuantumComputation circuit(4);
  circuit.cx(1, 0);
  circuit.cx(2, 1);
  circuit.cx(3, 2);
  circuit.cx(1, 2);
  circuit.cx(1, 3);
  circuit.cx(2, 3);
  circuit.cx(0, 1);
  circuit.cx(0, 3);

  const qc::Permutation identity = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), identity);
}

TEST(DDMinimizerTest, OrdersTargetsBeforeControlsInFallback) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(1, 3);

  const qc::Permutation expected = {{0, 2}, {1, 3}, {2, 1}, {3, 0}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), expected);
}

TEST(DDMinimizerTest, PreservesIdentityLayoutForCyclicFallback) {
  qc::QuantumComputation circuit(3);
  circuit.cx(0, 1);
  circuit.cx(1, 0);

  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit),
            circuit.initialLayout);
}

TEST(DDMinimizerTest, IgnoresControlledZOperationsInFallback) {
  qc::QuantumComputation circuit(3);
  circuit.cx(0, 2);
  circuit.cz(2, 0);

  const qc::Permutation expected = {{0, 1}, {1, 2}, {2, 0}};
  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit), expected);
}

TEST(DDMinimizerTest, NormalizesExistingSparseLayoutBeforeOptimization) {
  qc::QuantumComputation canonicalCircuit(4);
  canonicalCircuit.cx(0, 1);
  canonicalCircuit.cx(1, 2);
  canonicalCircuit.cx(2, 3);

  qc::QuantumComputation sparseCircuit;
  sparseCircuit.addQubit(0, 0, 0);
  sparseCircuit.addQubit(1, 2, 1);
  sparseCircuit.addQubit(2, 5, 2);
  sparseCircuit.addQubit(3, 7, 3);
  sparseCircuit.cx(0, 2);
  sparseCircuit.cx(2, 5);
  sparseCircuit.cx(5, 7);

  qc::DDMinimizer::optimizeInputPermutation(canonicalCircuit);
  qc::DDMinimizer::optimizeInputPermutation(sparseCircuit);

  EXPECT_EQ(sparseCircuit.initialLayout, canonicalCircuit.initialLayout);
  EXPECT_EQ(sparseCircuit.outputPermutation,
            canonicalCircuit.outputPermutation);
  ASSERT_EQ(sparseCircuit.size(), canonicalCircuit.size());
  auto canonicalOp = canonicalCircuit.cbegin();
  for (const auto& sparseOp : sparseCircuit) {
    EXPECT_EQ(*sparseOp, **canonicalOp);
    ++canonicalOp;
  }
}

TEST(DDMinimizerTest, LeavesAncillaryQubitsUnchanged) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.setLogicalQubitAncillary(0);
  const qc::QuantumComputation original = circuit;

  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit),
            circuit.initialLayout);
  qc::DDMinimizer::optimizeInputPermutation(circuit);

  expectCircuitUnchanged(circuit, original);
}

TEST(DDMinimizerTest, LeavesGarbageQubitsUnchanged) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(2, 3);
  circuit.setLogicalQubitGarbage(1);
  const qc::QuantumComputation original = circuit;

  EXPECT_EQ(qc::DDMinimizer::createGateBasedPermutation(circuit),
            circuit.initialLayout);
  qc::DDMinimizer::optimizeInputPermutation(circuit);

  expectCircuitUnchanged(circuit, original);
}

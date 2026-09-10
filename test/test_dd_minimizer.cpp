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
#include "ir/operations/Control.hpp"

#include <gtest/gtest.h>

TEST(DDMinimizerTest, OrdersTargetsBelowControls) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 3);
  circuit.cx(3, 2);
  const auto original = circuit;

  ddsim::DDMinimizer::optimizeInputPermutation(circuit);

  EXPECT_EQ(circuit.initialLayout,
            (qc::Permutation{{0, 3}, {1, 2}, {2, 0}, {3, 1}}));
  for (const auto& op : circuit) {
    EXPECT_LT(circuit.initialLayout.at(op->getTargets().front()),
              circuit.initialLayout.at(op->getControls().begin()->qubit));
  }
  auto expected = original;
  expected.initialLayout = circuit.initialLayout;
  EXPECT_EQ(circuit, expected);
  ddsim::DDMinimizer::optimizeInputPermutation(circuit);
  EXPECT_EQ(circuit, expected);
}

TEST(DDMinimizerTest, DeduplicatesDependenciesAndPreservesTieOrder) {
  qc::QuantumComputation circuit(4);
  circuit.cx(0, 1);
  circuit.cx(1, 2);
  circuit.cx(1, 3);
  circuit.cx(1, 3);

  EXPECT_EQ(ddsim::DDMinimizer::createGateBasedPermutation(circuit),
            (qc::Permutation{{0, 3}, {1, 2}, {2, 0}, {3, 1}}));
}

TEST(DDMinimizerTest, PreservesLayoutForCycles) {
  qc::QuantumComputation circuit(3);
  circuit.initialLayout = {{0, 1}, {1, 2}, {2, 0}};
  circuit.cx(0, 1);
  circuit.cx(1, 0);
  const auto original = circuit;

  ddsim::DDMinimizer::optimizeInputPermutation(circuit);

  EXPECT_EQ(circuit, original);
}

TEST(DDMinimizerTest, IgnoresControlledZ) {
  qc::QuantumComputation circuit(3);
  circuit.cx(0, 2);
  circuit.cz(2, 0);

  EXPECT_EQ(ddsim::DDMinimizer::createGateBasedPermutation(circuit),
            (qc::Permutation{{0, 2}, {1, 0}, {2, 1}}));
}

TEST(DDMinimizerTest, IncludesNestedCompoundDependencies) {
  qc::QuantumComputation flat(4);
  flat.cx(0, 1);
  flat.cx(1, 2);
  flat.cx(2, 3);
  auto inner = flat;
  qc::QuantumComputation outer(4);
  outer.emplace_back(inner.asCompoundOperation());
  qc::QuantumComputation nested(4);
  nested.emplace_back(outer.asCompoundOperation());
  auto expected = nested;
  expected.initialLayout = ddsim::DDMinimizer::createGateBasedPermutation(flat);

  ddsim::DDMinimizer::optimizeInputPermutation(nested);

  EXPECT_EQ(nested, expected);
  EXPECT_EQ(nested.initialLayout,
            (qc::Permutation{{0, 3}, {1, 2}, {2, 1}, {3, 0}}));
}

TEST(DDMinimizerTest, PreservesSparseRegistersAndOperations) {
  qc::QuantumComputation circuit;
  circuit.addQubit(0, 0, 0);
  circuit.addQubit(1, 2, 1);
  circuit.addQubit(2, 5, 2);
  circuit.addQubit(3, 7, 3);
  circuit.cx(0, 2);
  circuit.cx(2, 5);
  circuit.cx(5, 7);
  auto expected = circuit;
  expected.initialLayout = {{0, 3}, {2, 2}, {5, 1}, {7, 0}};

  ddsim::DDMinimizer::optimizeInputPermutation(circuit);

  EXPECT_EQ(circuit, expected);
  EXPECT_FALSE(circuit.toQASM().empty());
}

TEST(DDMinimizerTest, KeepsSwapsAndMeasurements) {
  qc::QuantumComputation circuit(3, 3);
  circuit.cx(0, 2);
  circuit.swap(0, 1);
  circuit.measure(0, 2);
  auto expected = circuit;
  expected.initialLayout = {{0, 2}, {1, 0}, {2, 1}};

  ddsim::DDMinimizer::optimizeInputPermutation(circuit);

  EXPECT_EQ(circuit, expected);
}

TEST(DDMinimizerTest, HandlesEmptyAndSingleQubitCircuits) {
  qc::QuantumComputation empty;
  EXPECT_TRUE(ddsim::DDMinimizer::createGateBasedPermutation(empty).empty());
  ddsim::DDMinimizer::optimizeInputPermutation(empty);
  EXPECT_EQ(empty, qc::QuantumComputation{});

  qc::QuantumComputation single(1);
  single.x(0);
  const auto original = single;
  ddsim::DDMinimizer::optimizeInputPermutation(single);
  EXPECT_EQ(single, original);
}

TEST(DDMinimizerTest, LeavesLargeUncontrolledCircuitsUnchanged) {
  qc::QuantumComputation circuit(4096);
  circuit.h(0);
  const auto original = circuit;

  ddsim::DDMinimizer::optimizeInputPermutation(circuit);

  EXPECT_EQ(circuit, original);
}

TEST(DDMinimizerTest, LeavesAncillaryAndGarbageQubitsUnchanged) {
  for (const bool ancillary : {false, true}) {
    qc::QuantumComputation circuit(4);
    circuit.cx(0, 1);
    circuit.cx(1, 2);
    circuit.cx(2, 3);
    if (ancillary) {
      circuit.setLogicalQubitAncillary(0);
    } else {
      circuit.setLogicalQubitGarbage(1);
    }
    const auto original = circuit;

    ddsim::DDMinimizer::optimizeInputPermutation(circuit);

    EXPECT_EQ(circuit, original);
  }
}

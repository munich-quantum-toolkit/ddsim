/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "UnitarySimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include <cstddef>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

using namespace qc::literals;

namespace {
void expectConstructionModesEquivalent(
    const qc::QuantumComputation& quantumComputation) {
  auto sequentialCircuit =
      std::make_unique<qc::QuantumComputation>(quantumComputation);
  auto recursiveCircuit =
      std::make_unique<qc::QuantumComputation>(quantumComputation);
  UnitarySimulator sequential(std::move(sequentialCircuit),
                              UnitarySimulator::Mode::Sequential);
  UnitarySimulator recursive(std::move(recursiveCircuit),
                             UnitarySimulator::Mode::Recursive);

  ASSERT_NO_THROW(sequential.construct());
  ASSERT_NO_THROW(recursive.construct());

  const auto sequentialMatrix =
      sequential.getConstructedDD().getMatrix(quantumComputation.getNqubits());
  const auto recursiveDD = recursive.getConstructedDD();
  const auto recursiveMatrix =
      recursiveDD.getMatrix(quantumComputation.getNqubits());
  ASSERT_EQ(sequentialMatrix.size(), recursiveMatrix.size());
  for (std::size_t row = 0U; row < sequentialMatrix.size(); ++row) {
    ASSERT_EQ(sequentialMatrix[row].size(), recursiveMatrix[row].size());
    for (std::size_t column = 0U; column < sequentialMatrix[row].size();
         ++column) {
      EXPECT_NEAR(sequentialMatrix[row][column].real(),
                  recursiveMatrix[row][column].real(), 1e-12);
      EXPECT_NEAR(sequentialMatrix[row][column].imag(),
                  recursiveMatrix[row][column].imag(), 1e-12);
    }
  }

  if (!recursiveDD.isTerminal()) {
    const auto& roots = recursive.dd->getRootSet<dd::mNode>();
    ASSERT_EQ(roots.size(), 1U);
    EXPECT_EQ(roots.at(recursiveDD), 1U);
  }
}
} // namespace

TEST(UnitarySimTest, ConstructSimpleCircuitSequential) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(2);
  quantumComputation->ch(2, 1);
  quantumComputation->ch(2, 0);
  UnitarySimulator ddsim(std::move(quantumComputation),
                         UnitarySimulator::Mode::Sequential);
  ASSERT_NO_THROW(ddsim.construct());
  const auto& e = ddsim.getConstructedDD();
  EXPECT_TRUE(e.p->e[0].isIdentity());
  EXPECT_TRUE(e.p->e[1].isIdentity());
  auto finalNodes = ddsim.getFinalNodeCount();
  EXPECT_EQ(finalNodes, 4);
  auto constructionTime = ddsim.getConstructionTime();
  std::cout << "Construction took " << constructionTime << "s\n";
}

TEST(UnitarySimTest, ConstructSimpleCircuitRecursive) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(2);
  quantumComputation->ch(2, 1);
  quantumComputation->ch(2, 0);
  UnitarySimulator ddsim(std::move(quantumComputation),
                         UnitarySimulator::Mode::Recursive);
  ASSERT_NO_THROW(ddsim.construct());
  const auto& e = ddsim.getConstructedDD();
  EXPECT_TRUE(e.p->e[0].isIdentity());
  EXPECT_TRUE(e.p->e[1].isIdentity());
  auto finalNodes = ddsim.getFinalNodeCount();
  EXPECT_EQ(finalNodes, 4);
  auto constructionTime = ddsim.getConstructionTime();
  std::cout << "Construction took " << constructionTime << "s\n";
}

TEST(UnitarySimTest, ConstructSimpleCircuitRecursiveWithSeed) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(2);
  quantumComputation->ch(2, 1);
  quantumComputation->ch(2, 0);
  UnitarySimulator ddsim(std::move(quantumComputation), ApproximationInfo{},
                         1337, UnitarySimulator::Mode::Recursive);
  ASSERT_NO_THROW(ddsim.construct());
  const auto& e = ddsim.getConstructedDD();
  EXPECT_TRUE(e.p->e[0].isIdentity());
  EXPECT_TRUE(e.p->e[1].isIdentity());
}

TEST(UnitarySimTest, ConstructionModesEquivalentAtBoundaries) {
  {
    SCOPED_TRACE("empty circuit");
    const qc::QuantumComputation quantumComputation(2);
    expectConstructionModesEquivalent(quantumComputation);
  }

  {
    SCOPED_TRACE("single virtual swap with matching output permutation");
    qc::QuantumComputation quantumComputation(2);
    quantumComputation.swap(0, 1);
    quantumComputation.outputPermutation[0] = 1;
    quantumComputation.outputPermutation[1] = 0;
    expectConstructionModesEquivalent(quantumComputation);
  }

  {
    SCOPED_TRACE("ancillary and garbage reduction");
    qc::QuantumComputation quantumComputation(2);
    quantumComputation.x(0);
    quantumComputation.cx(0, 1);
    quantumComputation.setLogicalQubitAncillary(1);
    quantumComputation.setLogicalQubitGarbage(1);
    expectConstructionModesEquivalent(quantumComputation);
  }

  {
    SCOPED_TRACE("odd operation count with virtual swap and layouts");
    qc::QuantumComputation quantumComputation(2);
    quantumComputation.h(0);
    quantumComputation.swap(0, 1);
    quantumComputation.x(0);
    quantumComputation.cx(0, 1);
    quantumComputation.z(1);
    quantumComputation.initialLayout[0] = 1;
    quantumComputation.initialLayout[1] = 0;
    quantumComputation.outputPermutation[0] = 1;
    quantumComputation.outputPermutation[1] = 0;
    expectConstructionModesEquivalent(quantumComputation);
  }
}

TEST(UnitarySimTest, NonStandardOperation) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(1, 1);
  quantumComputation->h(0);
  quantumComputation->measure(0, 0);
  quantumComputation->barrier(0);
  quantumComputation->h(0);
  quantumComputation->measure(0, 0);

  UnitarySimulator ddsim(std::move(quantumComputation));
  EXPECT_TRUE(ddsim.getMode() == UnitarySimulator::Mode::Recursive);
  EXPECT_THROW(ddsim.construct(), std::invalid_argument);
}

/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DensityComputeTable.hpp"
#include "DensityDDPackage.hpp"
#include "DensityNode.hpp"
#include "StochasticNoiseOperationTable.hpp"
#include "dd/ComplexValue.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Operations.hpp"
#include "dd/Package.hpp"
#include "ir/Definitions.hpp"
#include "ir/operations/OpType.hpp"
#include "ir/operations/StandardOperation.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace {
/// Create a `dd::Package` configured for density-matrix simulation.
std::unique_ptr<dd::Package> makePackage(const std::size_t nqubits) {
  return std::make_unique<dd::Package>(
      nqubits, dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG);
}
} // namespace

TEST(DensityDDPackageTest, ApplyOperationToDensityYieldsUniformDistribution) {
  constexpr std::size_t nrQubits = 3U;
  const auto pkg = makePackage(nrQubits);
  dd::ddsim::DensityDDPackage densityDD(*pkg, nrQubits);

  auto state = densityDD.makeZeroDensityOperator(nrQubits);

  std::vector<dd::mEdge> operations{};
  operations.emplace_back(dd::getDD(qc::StandardOperation(0, qc::H), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(1, qc::H), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(2, qc::H), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(2, qc::Z), *pkg));

  for (const auto& op : operations) {
    densityDD.applyOperationToDensity(state, op);
  }

  // H on every qubit spreads the state uniformly; the additional Z only
  // changes signs, which are not observable in the probabilities.
  const auto probabilities =
      state.getSparseProbabilityVectorStrKeys(nrQubits, 0.001);
  EXPECT_EQ(probabilities.size(), 1U << nrQubits);
  static constexpr dd::fp TOLERANCE = 1e-10;
  for (const auto& [state_, probability] : probabilities) {
    EXPECT_NEAR(probability, 0.125, TOLERANCE);
  }
}

TEST(DensityDDPackageTest, TraceOfZeroDensityOperatorIsOne) {
  constexpr std::size_t nrQubits = 3U;
  const auto pkg = makePackage(nrQubits);
  dd::ddsim::DensityDDPackage densityDD(*pkg, nrQubits);

  const auto state = densityDD.makeZeroDensityOperator(nrQubits);

  const auto trace = densityDD.trace(state, nrQubits);
  static constexpr dd::fp TOLERANCE = 1e-10;
  EXPECT_NEAR(trace.r, 1., TOLERANCE);
  EXPECT_NEAR(trace.i, 0., TOLERANCE);
}

TEST(DensityDDPackageTest, ReferenceCountingKeepsNodesAliveDuringCollection) {
  constexpr std::size_t nrQubits = 2U;
  const auto pkg = makePackage(nrQubits);
  dd::ddsim::DensityDDPackage densityDD(*pkg, nrQubits);

  EXPECT_EQ(densityDD.computeActiveNodeCount(), 0U);

  // `makeZeroDensityOperator` returns a referenced DD with one node per qubit.
  auto state = densityDD.makeZeroDensityOperator(nrQubits);
  EXPECT_EQ(densityDD.computeActiveNodeCount(), nrQubits);

  // A referenced DD survives garbage collection.
  EXPECT_FALSE(densityDD.garbageCollect(true));
  EXPECT_EQ(densityDD.computeActiveNodeCount(), nrQubits);

  // An additional reference has to be released before the DD becomes inactive.
  densityDD.incRef(state);
  densityDD.decRef(state);
  EXPECT_EQ(densityDD.computeActiveNodeCount(), nrQubits);

  densityDD.decRef(state);
  EXPECT_EQ(densityDD.computeActiveNodeCount(), 0U);
  EXPECT_TRUE(densityDD.garbageCollect(true));
  EXPECT_EQ(densityDD.computeActiveNodeCount(), 0U);
}

TEST(DensityDDPackageTest, ComputeTableDiscriminatesDensityMatrixResults) {
  constexpr std::size_t nrQubits = 1U;
  const auto pkg = makePackage(nrQubits);
  dd::ddsim::DensityDDPackage densityDD(*pkg, nrQubits);

  const auto state = densityDD.makeZeroDensityOperator(nrQubits);
  const auto operation =
      dd::ddsim::densityFromMatrixEdge(pkg->conjugateTranspose(
          dd::getDD(qc::StandardOperation(0, qc::H), *pkg)));
  ASSERT_FALSE(state.isTerminal());

  dd::ddsim::DensityComputeTable<dd::ddsim::dNode*, dd::ddsim::dNode*,
                                 dd::ddsim::dCachedEdge>
      computeTable(16U);

  // Nothing is cached initially.
  EXPECT_EQ(computeTable.lookup(state.p, operation.p), nullptr);

  // The zero density operator is stored as a full matrix, ...
  ASSERT_FALSE(dd::ddsim::dNode::isDensityMatrixNode(state.p->flags));
  computeTable.insert(state.p, operation.p,
                      dd::ddsim::dCachedEdge{state.p, dd::ComplexValue(1.)});

  // ... so it may only be returned when a matrix is requested. Returning it
  // for a requested (reduced) density matrix would be incorrect.
  const auto* const matrixResult =
      computeTable.lookup(state.p, operation.p, false);
  ASSERT_NE(matrixResult, nullptr);
  EXPECT_EQ(matrixResult->p, state.p);
  EXPECT_EQ(computeTable.lookup(state.p, operation.p, true), nullptr);

  // The temporary density flags encoded in the node pointer are part of the
  // key, so a flagged operand must not hit the entry above.
  auto* flagged = state.p;
  dd::ddsim::dNode::setDensityMatTempFlagTrue(flagged);
  EXPECT_EQ(computeTable.lookup(flagged, operation.p, false), nullptr);

  computeTable.clear();
  EXPECT_EQ(computeTable.lookup(state.p, operation.p, false), nullptr);
}

TEST(StochasticNoiseOperationTableTest, InsertLookupAndClear) {
  constexpr std::size_t nrQubits = 4U;
  const auto pkg = makePackage(nrQubits);

  std::vector<dd::mEdge> operations{};
  operations.emplace_back(dd::getDD(qc::StandardOperation(0, qc::X), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(1, qc::Z), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(2, qc::Y), *pkg));
  operations.emplace_back(dd::getDD(qc::StandardOperation(3, qc::H), *pkg));

  dd::ddsim::StochasticNoiseOperationTable<dd::mEdge> table(nrQubits,
                                                            operations.size());

  // Cache the i-th operation for target qubit i.
  for (std::size_t i = 0; i < operations.size(); ++i) {
    table.insert(static_cast<std::uint8_t>(i), static_cast<qc::Qubit>(i),
                 operations[i]);
  }

  for (std::size_t i = 0; i < operations.size(); ++i) {
    for (qc::Qubit j = 0; j < nrQubits; ++j) {
      const auto* const op = table.lookup(static_cast<std::uint8_t>(i), j);
      if (static_cast<qc::Qubit>(i) == j) {
        ASSERT_NE(op, nullptr);
        EXPECT_EQ(op->p, operations[i].p);
      } else {
        EXPECT_EQ(op, nullptr);
      }
    }
  }

  table.clear();
  for (std::size_t i = 0; i < operations.size(); ++i) {
    for (qc::Qubit j = 0; j < nrQubits; ++j) {
      EXPECT_EQ(table.lookup(static_cast<std::uint8_t>(i), j), nullptr);
    }
  }
}

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

#include "CircuitSimulator.hpp"
#include "circuit_optimizer/CircuitOptimizer.hpp"
#include "dd/DDpackageConfig.hpp"
#include "dd/FunctionalityConstruction.hpp"
#include "dd/Node.hpp"
#include "dd/Operations.hpp"
#include "dd/Package.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/OpType.hpp"

#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace {
dd::MatrixDD buildFunctionalityPairwise(const qc::QuantumComputation& qc,
                                        const std::size_t begin,
                                        const std::size_t count,
                                        qc::Permutation& permutation,
                                        dd::Package& package) {
  if (count == 1U) {
    auto e = dd::Package::makeIdent();
    if (const auto& op = qc.at(begin);
        op->getType() == qc::OpType::SWAP && !op->isControlled()) {
      const auto& targets = op->getTargets();
      std::swap(permutation.at(targets[0U]), permutation.at(targets[1U]));
    } else {
      e = dd::getDD(*op, package, permutation);
    }
    package.incRef(e);
    return e;
  }

  const auto leftCount = std::bit_floor(count - 1U);
  const auto left =
      buildFunctionalityPairwise(qc, begin, leftCount, permutation, package);
  const auto right = buildFunctionalityPairwise(
      qc, begin + leftCount, count - leftCount, permutation, package);
  const auto result = package.multiply(right, left);
  package.incRef(result);
  package.decRef(left);
  package.decRef(right);
  package.garbageCollect();
  return result;
}

dd::MatrixDD buildFunctionalityPairwise(const qc::QuantumComputation& qc,
                                        dd::Package& package) {
  if (qc.getNqubits() == 0U || qc.size() < 2U) {
    return dd::buildFunctionality(qc, package);
  }

  auto permutation = qc.initialLayout;
  auto e = buildFunctionalityPairwise(qc, 0U, qc.size(), permutation, package);

  dd::changePermutation(e, permutation, qc.outputPermutation, package);
  e = package.reduceAncillae(e, qc.getAncillary());
  e = package.reduceGarbage(e, qc.getGarbage());
  return e;
}
} // namespace

void UnitarySimulator::construct() {
  // carry out actual computation
  auto start = std::chrono::steady_clock::now();
  if (mode == Mode::Sequential) {
    e = dd::buildFunctionality(*qc, *dd);
  } else if (mode == Mode::Recursive) {
    e = buildFunctionalityPairwise(*qc, *dd);
  }
  auto end = std::chrono::steady_clock::now();
  constructionTime = std::chrono::duration<double>(end - start).count();
}

UnitarySimulator::UnitarySimulator(
    std::unique_ptr<qc::QuantumComputation>&& qc_,
    const ApproximationInfo& approximationInfo_, Mode simMode)
    : CircuitSimulator(std::move(qc_), approximationInfo_,
                       dd::UNITARY_SIMULATOR_DD_PACKAGE_CONFIG),
      mode(simMode) {
  // remove final measurements
  qc::CircuitOptimizer::removeFinalMeasurements(*qc);
}

UnitarySimulator::UnitarySimulator(
    std::unique_ptr<qc::QuantumComputation>&& qc_, Mode simMode)
    : UnitarySimulator(std::move(qc_), {}, simMode) {}

UnitarySimulator::UnitarySimulator(
    std::unique_ptr<qc::QuantumComputation>&& qc_,
    const ApproximationInfo& approximationInfo_, const std::uint64_t seed_,
    const Mode simMode)
    : CircuitSimulator(std::move(qc_), approximationInfo_, seed_,
                       dd::UNITARY_SIMULATOR_DD_PACKAGE_CONFIG),
      mode(simMode) {
  // remove final measurements
  qc::CircuitOptimizer::removeFinalMeasurements(*qc);
}

/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
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
#include "ir/QuantumComputation.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

void UnitarySimulator::construct() {
  // carry out actual computation
  auto start = std::chrono::steady_clock::now();
  if (mode == Mode::Sequential) {
    e = dd::buildFunctionality(*qc, *dd);
  } else if (mode == Mode::Recursive) {
    e = dd::buildFunctionalityRecursive(*qc, *dd);
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

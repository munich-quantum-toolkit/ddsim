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

#include "CircuitSimulator.hpp"
#include "DensityDDPackage.hpp"
#include "DensityNode.hpp"
#include "NoiseFunctionality.hpp"
#include "Simulator.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Package.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/NonUnitaryOperation.hpp"
#include "ir/operations/Operation.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

class DeterministicNoiseSimulator : public CircuitSimulator {
public:
  DeterministicNoiseSimulator(
      std::unique_ptr<qc::QuantumComputation>&& qc_,
      const ApproximationInfo& approximationInfo_,
      std::string noiseEffects_ = "APD", double noiseProbability_ = 0.001,
      std::optional<double> ampDampingProbability_ = std::nullopt,
      double multiQubitGateFactor_ = 2)
      : CircuitSimulator(std::move(qc_), approximationInfo_,
                         dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG),
        noiseEffects(std::move(noiseEffects_)),
        noiseProbSingleQubit(noiseProbability_),
        ampDampingProbSingleQubit(ampDampingProbability_
                                      ? ampDampingProbability_.value()
                                      : noiseProbability_ * 2),
        noiseProbMultiQubit(noiseProbability_ * multiQubitGateFactor_),
        ampDampingProbMultiQubit(ampDampingProbSingleQubit *
                                 multiQubitGateFactor_),
        densityDD(*dd, CircuitSimulator::getNumberOfQubits()),
        deterministicNoiseFunctionality(
            densityDD, CircuitSimulator::getNumberOfQubits(),
            noiseProbSingleQubit, noiseProbMultiQubit,
            ampDampingProbSingleQubit, ampDampingProbMultiQubit, noiseEffects) {
    dd::ddsim::sanityCheckOfNoiseProbabilities(
        noiseProbability_, ampDampingProbSingleQubit, multiQubitGateFactor_);
  }

  explicit DeterministicNoiseSimulator(
      std::unique_ptr<qc::QuantumComputation>&& qc_,
      const std::string& noiseEffects_ = "APD",
      double noiseProbability_ = 0.001,
      std::optional<double> ampDampingProbability_ = std::nullopt,
      double multiQubitGateFactor_ = 2)
      : DeterministicNoiseSimulator(std::move(qc_), {}, noiseEffects_,
                                    noiseProbability_, ampDampingProbability_,
                                    multiQubitGateFactor_) {}

  DeterministicNoiseSimulator(
      std::unique_ptr<qc::QuantumComputation>&& qc_,
      const ApproximationInfo& approximationInfo_, const std::size_t seed_,
      std::string noiseEffects_ = "APD", double noiseProbability_ = 0.001,
      std::optional<double> ampDampingProbability_ = std::nullopt,
      double multiQubitGateFactor_ = 2)
      : CircuitSimulator(std::move(qc_), approximationInfo_, seed_,
                         dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG),
        noiseEffects(std::move(noiseEffects_)),
        noiseProbSingleQubit(noiseProbability_),
        ampDampingProbSingleQubit(ampDampingProbability_
                                      ? ampDampingProbability_.value()
                                      : noiseProbability_ * 2),
        noiseProbMultiQubit(noiseProbability_ * multiQubitGateFactor_),
        ampDampingProbMultiQubit(ampDampingProbSingleQubit *
                                 multiQubitGateFactor_),
        densityDD(*dd, CircuitSimulator::getNumberOfQubits()),
        deterministicNoiseFunctionality(
            densityDD, CircuitSimulator::getNumberOfQubits(),
            noiseProbSingleQubit, noiseProbMultiQubit,
            ampDampingProbSingleQubit, ampDampingProbMultiQubit, noiseEffects) {
    dd::ddsim::sanityCheckOfNoiseProbabilities(
        noiseProbability_, ampDampingProbSingleQubit, multiQubitGateFactor_);
  }

  std::map<std::string, std::size_t>
  measureAllNonCollapsing(std::size_t shots) override {
    return sampleFromProbabilityMap(
        rootEdge.getSparseProbabilityVectorStrKeys(getNumberOfQubits(),
                                                   measurementThreshold),
        shots);
  }

  std::map<std::string, std::size_t> simulate(std::size_t shots) override {
    return simulateWithHooks(shots);
  }

  void initializeSimulation(std::size_t nQubits) override;
  char measure(dd::Qubit i) override;
  void reset(qc::NonUnitaryOperation* nonUnitaryOp) override;
  void applyOperationToState(std::unique_ptr<qc::Operation>& op) override;

  std::map<std::string, std::size_t>
  sampleFromProbabilityMap(const dd::SparsePVecStrKeys& resultProbabilityMap,
                           std::size_t shots);

  [[nodiscard]] std::size_t getActiveNodeCount() const override {
    return densityDD.computeActiveNodeCount();
  }

  [[nodiscard]] std::size_t countNodesFromRoot() override {
    dd::ddsim::DensityMatrixDD::alignDensityEdge(rootEdge);
    const std::size_t tmp = rootEdge.size();
    dd::ddsim::DensityMatrixDD::setDensityMatrixTrue(rootEdge);
    return tmp;
  }

  dd::ddsim::DensityMatrixDD rootEdge{};

private:
  std::string noiseEffects;

  double noiseProbSingleQubit{};
  double ampDampingProbSingleQubit{};
  double noiseProbMultiQubit{};
  double ampDampingProbMultiQubit{};

  double measurementThreshold = 0.01;
  dd::ddsim::DensityDDPackage densityDD;
  dd::ddsim::DeterministicNoiseFunctionality deterministicNoiseFunctionality;
  bool densityStateInitialized = false;
};

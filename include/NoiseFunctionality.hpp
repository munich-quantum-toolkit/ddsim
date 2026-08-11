/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

/**
 * @file NoiseFunctionality.hpp
 * @brief Stochastic and deterministic noise functionality.
 *
 * @details Self-contained copy of the MQT Core `dd::NoiseFunctionality` that
 * was removed alongside the density-matrix support. The stochastic noise
 * operation cache is now owned by @ref StochasticNoiseFunctionality (instead of
 * the DD package), and the removed noise `qc::OpType`s
 * (`ATrue`/`AFalse`/`MultiATrue`/`MultiAFalse`) are replaced by a local
 * enumeration used only for cache keys and noise-operation selection.
 */

#pragma once

#include "DensityDDPackage.hpp"
#include "DensityNode.hpp"
#include "StochasticNoiseOperationTable.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Package.hpp"
#include "ir/Definitions.hpp"
#include "ir/operations/Operation.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace dd::ddsim {

using NrEdges = std::tuple_size<decltype(dNode::e)>;
using ArrayOfEdges = std::array<dCachedEdge, NrEdges::value>;

// noise operations available for deterministic noise aware quantum circuit
// simulation
enum NoiseOperations : std::uint8_t {
  AmplitudeDamping,
  PhaseFlip,
  Depolarization,
  Identity
};

void sanityCheckOfNoiseProbabilities(double noiseProbability,
                                     double amplitudeDampingProb,
                                     double multiQubitGateFactor);

class StochasticNoiseFunctionality {
public:
  StochasticNoiseFunctionality(dd::Package& dd, std::size_t nq,
                               double gateNoiseProbability,
                               double amplitudeDampingProb,
                               double multiQubitGateFactor,
                               const std::string& cNoiseEffects);

  ~StochasticNoiseFunctionality() { package->decRef(identityDD); }

protected:
  /// Local identifiers for cached stochastic noise operations, replacing the
  /// removed noise `qc::OpType`s.
  enum StochasticNoiseKind : std::uint8_t {
    StochX,
    StochY,
    StochZ,
    StochATrue,
    StochAFalse,
    StochMultiATrue,
    StochMultiAFalse,
    StochIdentity,
    StochasticNoiseKindEnd
  };

  dd::Package* package;
  std::size_t nQubits;
  std::uniform_real_distribution<dd::fp> dist;

  double noiseProbability;
  double noiseProbabilityMulti;
  dd::fp sqrtAmplitudeDampingProbability;
  dd::fp oneMinusSqrtAmplitudeDampingProbability;
  dd::fp sqrtAmplitudeDampingProbabilityMulti;
  dd::fp oneMinusSqrtAmplitudeDampingProbabilityMulti;
  dd::GateMatrix ampDampingTrue{};
  dd::GateMatrix ampDampingTrueMulti{};
  dd::GateMatrix ampDampingFalse{};
  dd::GateMatrix ampDampingFalseMulti{};
  std::vector<NoiseOperations> noiseEffects;
  dd::mEdge identityDD;
  StochasticNoiseOperationTable<dd::mEdge> stochasticNoiseOperationCache;

  [[nodiscard]] std::size_t getNumberOfQubits() const { return nQubits; }
  [[nodiscard]] double getNoiseProbability(bool multiQubitNoiseFlag) const;

  [[nodiscard]] static StochasticNoiseKind
  getAmplitudeDampingOperationType(bool multiQubitNoiseFlag,
                                   bool amplitudeDampingFlag);

  [[nodiscard]] dd::GateMatrix
  getAmplitudeDampingOperationMatrix(bool multiQubitNoiseFlag,
                                     bool amplitudeDampingFlag) const;

public:
  void applyNoiseOperation(const std::set<qc::Qubit>& targets,
                           dd::mEdge operation, dd::vEdge& state,
                           std::mt19937_64& generator);

protected:
  [[nodiscard]] dd::mEdge stackOperation(const dd::mEdge& operation,
                                         qc::Qubit target,
                                         StochasticNoiseKind noiseOperation,
                                         const dd::GateMatrix& matrix);

  dd::mEdge generateNoiseOperation(dd::mEdge operation, qc::Qubit target,
                                   std::mt19937_64& generator,
                                   bool amplitudeDamping,
                                   bool multiQubitOperation);

  [[nodiscard]] StochasticNoiseKind
  returnNoiseOperation(NoiseOperations noiseOperation, double prob,
                       bool multiQubitNoiseFlag) const;
};

class DeterministicNoiseFunctionality {
public:
  DeterministicNoiseFunctionality(DensityDDPackage& dd, std::size_t nq,
                                  double noiseProbabilitySingleQubit,
                                  double noiseProbabilityMultiQubit,
                                  double ampDampProbSingleQubit,
                                  double ampDampProbMultiQubit,
                                  const std::string& cNoiseEffects);

protected:
  DensityDDPackage* package;
  std::size_t nQubits;

  double noiseProbSingleQubit;
  double noiseProbMultiQubit;
  double ampDampingProbSingleQubit;
  double ampDampingProbMultiQubit;

  std::vector<NoiseOperations> noiseEffects;

  [[nodiscard]] std::size_t getNumberOfQubits() const { return nQubits; }

public:
  void applyNoiseEffects(dEdge& originalEdge,
                         const std::unique_ptr<qc::Operation>& qcOperation);

private:
  dCachedEdge applyNoiseEffects(dEdge& originalEdge,
                                const std::set<qc::Qubit>& usedQubits,
                                bool firstPathEdge, dd::Qubit level);

  static void applyPhaseFlipToEdges(ArrayOfEdges& e, double probability);

  void applyAmplitudeDampingToEdges(ArrayOfEdges& e, double probability) const;

  void applyDepolarisationToEdges(ArrayOfEdges& e, double probability) const;
};

} // namespace dd::ddsim

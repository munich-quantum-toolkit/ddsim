/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DensityDDPackage.hpp"
#include "DensityNode.hpp"
#include "NoiseFunctionality.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Operations.hpp"
#include "dd/Package.hpp"
#include "dd/StateGeneration.hpp"
#include "ir/QuantumComputation.hpp"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <complex>
#include <cstddef>
#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

class DDNoiseFunctionalityTest : public ::testing::Test {
protected:
  void SetUp() override {
    // circuit taken from https://github.com/pnnl/qasmbench
    qc.addQubitRegister(4U);
    qc.x(0);
    qc.x(1);
    qc.h(3);
    qc.cx(2, 3);
    qc.t(0);
    qc.t(1);
    qc.t(2);
    qc.tdg(3);
    qc.cx(0, 1);
    qc.cx(2, 3);
    qc.cx(3, 0);
    qc.cx(1, 2);
    qc.cx(0, 1);
    qc.cx(2, 3);
    qc.tdg(0);
    qc.tdg(1);
    qc.tdg(2);
    qc.t(3);
    qc.cx(0, 1);
    qc.cx(2, 3);
    qc.s(3);
    qc.cx(3, 0);
    qc.h(3);
  }

  qc::QuantumComputation qc;
  std::mt19937_64 rng{0U};
  std::size_t stochRuns = 1000U;
};

TEST_F(DDNoiseFunctionalityTest, DetSimulateAdder4TrackAPD) {
  const dd::SparsePVecStrKeys reference = {
      {"0000", 0.0969332192741}, {"1000", 0.0907888041538},
      {"0100", 0.0141409660985}, {"1100", 0.0092413539333},
      {"0010", 0.0238203475524}, {"1010", 0.0235097990017},
      {"0110", 0.0244576087400}, {"1110", 0.0116282811276},
      {"0001", 0.1731941264570}, {"1001", 0.4145855071998},
      {"0101", 0.0138062113213}, {"1101", 0.0184033482066},
      {"0011", 0.0242454336917}, {"1011", 0.0262779844799},
      {"0111", 0.0239296920989}, {"1111", 0.0110373166627}};

  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG);
  dd::ddsim::DensityDDPackage densityDD(*dd, qc.getNqubits());

  auto rootEdge = densityDD.makeZeroDensityOperator(qc.getNqubits());

  const auto* const noiseEffects = "APDI";

  auto deterministicNoiseFunctionality =
      dd::ddsim::DeterministicNoiseFunctionality(
          densityDD, qc.getNqubits(), 0.01, 0.02, 0.02, 0.04, noiseEffects);

  for (auto const& op : qc) {
    densityDD.applyOperationToDensity(rootEdge, dd::getDD(*op, *dd));
    deterministicNoiseFunctionality.applyNoiseEffects(rootEdge, op);
  }

  // Expect that all results are the same
  const auto m =
      rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits(), 0.001);
  static constexpr dd::fp TOLERANCE = 1e-10;
  for (const auto& [key, value] : m) {
    EXPECT_NEAR(value, reference.at(key), TOLERANCE);
  }
}

TEST_F(DDNoiseFunctionalityTest, DetSimulateAdder4TrackD) {
  const dd::SparsePVecStrKeys reference = {
      {"0000", 0.0332328704931}, {"0001", 0.0683938280189},
      {"0011", 0.0117061689898}, {"0100", 0.0129643065735},
      {"0101", 0.0107812802908}, {"0111", 0.0160082331009},
      {"1000", 0.0328434857577}, {"1001", 0.7370101351171},
      {"1011", 0.0186346925411}, {"1101", 0.0275086747656}};

  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG);
  dd::ddsim::DensityDDPackage densityDD(*dd, qc.getNqubits());

  auto rootEdge = densityDD.makeZeroDensityOperator(qc.getNqubits());

  const auto* const noiseEffects = "D";

  auto deterministicNoiseFunctionality =
      dd::ddsim::DeterministicNoiseFunctionality(
          densityDD, qc.getNqubits(), 0.01, 0.02, 0.02, 0.04, noiseEffects);

  for (auto const& op : qc) {
    densityDD.applyOperationToDensity(rootEdge, dd::getDD(*op, *dd));
    deterministicNoiseFunctionality.applyNoiseEffects(rootEdge, op);
  }

  // Expect that all results are the same
  const auto m =
      rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits(), 0.01);
  static constexpr dd::fp TOLERANCE = 1e-10;
  for (const auto& [key, value] : m) {
    EXPECT_NEAR(value, reference.at(key), TOLERANCE);
  }
}

TEST_F(DDNoiseFunctionalityTest, testingMeasure) {
  constexpr double tolerance = 1e-10;

  qc::QuantumComputation qcOp{};
  qcOp.addQubitRegister(3U);
  qcOp.h(0);
  qcOp.h(1);
  qcOp.h(2);

  auto dd = std::make_unique<dd::Package>(
      qcOp.getNqubits(), dd::ddsim::DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG);
  dd::ddsim::DensityDDPackage densityDD(*dd, qcOp.getNqubits());

  auto rootEdge = densityDD.makeZeroDensityOperator(qcOp.getNqubits());

  auto deterministicNoiseFunctionality =
      dd::ddsim::DeterministicNoiseFunctionality(densityDD, qcOp.getNqubits(),
                                                 0.01, 0.02, 0.02, 0.04, {});

  for (auto const& op : qcOp) {
    densityDD.applyOperationToDensity(rootEdge, dd::getDD(*op, *dd));
    deterministicNoiseFunctionality.applyNoiseEffects(rootEdge, op);
  }

  auto tmp = rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits());
  auto prob = 0.125;
  EXPECT_NEAR(tmp["000"], prob, tolerance);
  EXPECT_NEAR(tmp["001"], prob, tolerance);
  EXPECT_NEAR(tmp["010"], prob, tolerance);
  EXPECT_NEAR(tmp["011"], prob, tolerance);
  EXPECT_NEAR(tmp["100"], prob, tolerance);
  EXPECT_NEAR(tmp["101"], prob, tolerance);
  EXPECT_NEAR(tmp["110"], prob, tolerance);
  EXPECT_NEAR(tmp["111"], prob, tolerance);

  densityDD.measureOneCollapsing(rootEdge, 0, rng);

  auto tmp0 = rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits());
  prob = 0.25;

  EXPECT_TRUE(std::fabs(tmp0["000"] + tmp0["001"] - prob) < tolerance);
  EXPECT_TRUE(std::fabs(tmp0["010"] + tmp0["011"] - prob) < tolerance);
  EXPECT_TRUE(std::fabs(tmp0["100"] + tmp0["101"] - prob) < tolerance);
  EXPECT_TRUE(std::fabs(tmp0["110"] + tmp0["111"] - prob) < tolerance);

  densityDD.measureOneCollapsing(rootEdge, 1, rng);

  auto tmp1 = rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits());
  prob = 0.5;
  EXPECT_TRUE(std::fabs(tmp0["000"] + tmp0["001"] + tmp0["010"] + tmp0["011"] -
                        prob) < tolerance);
  EXPECT_TRUE(std::fabs(tmp0["100"] + tmp0["101"] + tmp0["110"] + tmp0["111"] -
                        prob) < tolerance);

  densityDD.measureOneCollapsing(rootEdge, 2, rng);

  auto tmp2 = rootEdge.getSparseProbabilityVectorStrKeys(qc.getNqubits());
  EXPECT_TRUE(std::fabs(tmp2["000"] - 1) < tolerance ||
              std::fabs(tmp2["001"] - 1) < tolerance ||
              std::fabs(tmp2["010"] - 1) < tolerance ||
              std::fabs(tmp2["011"] - 1) < tolerance ||
              std::fabs(tmp2["100"] - 1) < tolerance ||
              std::fabs(tmp2["101"] - 1) < tolerance ||
              std::fabs(tmp2["111"] - 1) < tolerance);
}

TEST_F(DDNoiseFunctionalityTest, StochSimulateAdder4TrackAPD) {
  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::STOCHASTIC_NOISE_SIMULATOR_DD_PACKAGE_CONFIG);

  std::map<std::string, double, std::less<>> measSummary = {
      {"0000", 0.}, {"0001", 0.}, {"0010", 0.}, {"0011", 0.}, {"0100", 0.},
      {"0101", 0.}, {"0110", 0.}, {"0111", 0.}, {"1000", 0.}, {"1001", 0.},
      {"1010", 0.}, {"1011", 0.}, {"1100", 0.}, {"1101", 0.}};

  const auto* const noiseEffects = "APDI";

  auto stochasticNoiseFunctionality = dd::ddsim::StochasticNoiseFunctionality(
      *dd, qc.getNqubits(), 0.01, 0.02, 2., noiseEffects);

  for (std::size_t i = 0U; i < stochRuns; i++) {
    auto rootEdge = dd::makeZeroState(qc.getNqubits(), *dd);
    dd->incRef(rootEdge);

    for (auto const& op : qc) {
      auto operation = dd::getDD(*op, *dd);
      auto usedQubits = op->getUsedQubits();
      stochasticNoiseFunctionality.applyNoiseOperation(usedQubits, operation,
                                                       rootEdge, rng);
    }

    const auto amplitudes = rootEdge.getVector();
    for (std::size_t m = 0U; m < amplitudes.size(); m++) {
      auto state = std::bitset<4U>(m).to_string();
      std::ranges::reverse(state);
      const auto amplitude = amplitudes[m];
      const auto prob = std::norm(amplitude);
      measSummary[state] += prob / static_cast<double>(stochRuns);
    }
  }

  const double tolerance = 0.1;
  EXPECT_NEAR(measSummary["0000"], 0.09693321927412533, tolerance);
  EXPECT_NEAR(measSummary["0001"], 0.09078880415385877, tolerance);
  EXPECT_NEAR(measSummary["0010"], 0.01414096609854787, tolerance);
  EXPECT_NEAR(measSummary["0100"], 0.02382034755245074, tolerance);
  EXPECT_NEAR(measSummary["0101"], 0.023509799001774703, tolerance);
  EXPECT_NEAR(measSummary["0110"], 0.02445760874001203, tolerance);
  EXPECT_NEAR(measSummary["0111"], 0.011628281127642115, tolerance);
  EXPECT_NEAR(measSummary["1000"], 0.1731941264570172, tolerance);
  EXPECT_NEAR(measSummary["1001"], 0.41458550719988047, tolerance);
  EXPECT_NEAR(measSummary["1010"], 0.013806211321349706, tolerance);
  EXPECT_NEAR(measSummary["1011"], 0.01840334820660922, tolerance);
  EXPECT_NEAR(measSummary["1100"], 0.024245433691737584, tolerance);
  EXPECT_NEAR(measSummary["1101"], 0.026277984479993615, tolerance);
  EXPECT_NEAR(measSummary["1110"], 0.023929692098939092, tolerance);
  EXPECT_NEAR(measSummary["1111"], 0.011037316662706232, tolerance);
}

TEST_F(DDNoiseFunctionalityTest, StochSimulateAdder4IdentityError) {
  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::STOCHASTIC_NOISE_SIMULATOR_DD_PACKAGE_CONFIG);

  std::map<std::string, double, std::less<>> measSummary = {
      {"0000", 0.}, {"0001", 0.}, {"0010", 0.}, {"0011", 0.}, {"0100", 0.},
      {"0101", 0.}, {"0110", 0.}, {"0111", 0.}, {"1000", 0.}, {"1001", 0.},
      {"1010", 0.}, {"1011", 0.}, {"1100", 0.}, {"1101", 0.}};

  const auto* const noiseEffects = "I";

  auto stochasticNoiseFunctionality = dd::ddsim::StochasticNoiseFunctionality(
      *dd, qc.getNqubits(), 0.01, 0.02, 2., noiseEffects);

  for (std::size_t i = 0U; i < stochRuns; i++) {
    auto rootEdge = dd::makeZeroState(qc.getNqubits(), *dd);
    dd->incRef(rootEdge);

    for (auto const& op : qc) {
      auto operation = dd::getDD(*op, *dd);
      stochasticNoiseFunctionality.applyNoiseOperation(
          op->getUsedQubits(), operation, rootEdge, rng);
    }

    const auto amplitudes = rootEdge.getVector();
    for (std::size_t m = 0U; m < amplitudes.size(); m++) {
      auto state = std::bitset<4U>(m).to_string();
      std::ranges::reverse(state);
      const auto amplitude = amplitudes[m];
      const auto prob = std::norm(amplitude);
      measSummary[state] += prob / static_cast<double>(stochRuns);
    }
  }

  const double tolerance = 0.1;
  EXPECT_NEAR(measSummary["0000"], 0., tolerance);
  EXPECT_NEAR(measSummary["0001"], 0., tolerance);
  EXPECT_NEAR(measSummary["0010"], 0., tolerance);
  EXPECT_NEAR(measSummary["0100"], 0., tolerance);
  EXPECT_NEAR(measSummary["0101"], 0., tolerance);
  EXPECT_NEAR(measSummary["0110"], 0., tolerance);
  EXPECT_NEAR(measSummary["0111"], 0., tolerance);
  EXPECT_NEAR(measSummary["1000"], 0., tolerance);
  EXPECT_NEAR(measSummary["1001"], 1., tolerance);
  EXPECT_NEAR(measSummary["1010"], 0., tolerance);
  EXPECT_NEAR(measSummary["1011"], 0., tolerance);
  EXPECT_NEAR(measSummary["1100"], 0., tolerance);
  EXPECT_NEAR(measSummary["1101"], 0., tolerance);
  EXPECT_NEAR(measSummary["1110"], 0., tolerance);
  EXPECT_NEAR(measSummary["1111"], 0., tolerance);
}

TEST_F(DDNoiseFunctionalityTest, invalidNoiseEffect) {
  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::STOCHASTIC_NOISE_SIMULATOR_DD_PACKAGE_CONFIG);
  EXPECT_THROW(dd::ddsim::StochasticNoiseFunctionality(*dd, qc.getNqubits(),
                                                       0.01, 0.02, 2., "APK"),
               std::runtime_error);
}

TEST_F(DDNoiseFunctionalityTest, invalidNoiseProbabilities) {
  auto dd = std::make_unique<dd::Package>(
      qc.getNqubits(), dd::ddsim::STOCHASTIC_NOISE_SIMULATOR_DD_PACKAGE_CONFIG);
  EXPECT_THROW(dd::ddsim::StochasticNoiseFunctionality(*dd, qc.getNqubits(),
                                                       0.3, 0.6, 2, "APD"),
               std::runtime_error);
}

/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "CircuitSimulator.hpp"
#include "DeterministicNoiseSimulator.hpp"
#include "dd/Node.hpp"
#include "ir/Definitions.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/NonUnitaryOperation.hpp"
#include "ir/operations/OpType.hpp"
#include "ir/operations/Operation.hpp"

#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

class HookedCircuitSimulator final : public CircuitSimulator {
public:
  // Inherited constructors forward their arguments.
  // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
  using CircuitSimulator::CircuitSimulator;

  [[nodiscard]] bool operationHookWasCalled() const noexcept {
    return hookWasCalled;
  }

protected:
  void
  applyOperationToState(std::unique_ptr<qc::Operation>& operation) override {
    hookWasCalled = true;
    CircuitSimulator::applyOperationToState(operation);
  }

private:
  bool hookWasCalled = false;
};

} // namespace

TEST(CircuitExecutionTest, RetainsCanonicalStateAfterVirtualSwap) {
  auto circuit = std::make_unique<qc::QuantumComputation>(2U, 2U);
  circuit->x(0U);
  circuit->swap(0U, 1U);
  circuit->measure(0U, 0U);
  circuit->measure(1U, 1U);
  circuit->gphase(qc::PI_2);

  CircuitSimulator simulator(std::move(circuit), 17U);
  const auto counts = simulator.simulate(8U);

  EXPECT_EQ(counts, (std::map<std::string, std::size_t>{{"10", 8U}}));
  EXPECT_EQ(simulator.additionalStatistics().at("single_shots"), "1");
  const auto amplitudes = simulator.getCurrentDD().getVector();
  ASSERT_EQ(amplitudes.size(), 4U);
  EXPECT_NEAR(amplitudes.at(2U).real(), 0., 1e-12);
  EXPECT_NEAR(amplitudes.at(2U).imag(), 1., 1e-12);
}

TEST(CircuitExecutionTest, ExecutesDynamicMeasurementResetAndControl) {
  auto circuit = std::make_unique<qc::QuantumComputation>(2U, 2U);
  circuit->x(0U);
  circuit->swap(0U, 1U);
  circuit->measure(1U, 0U);
  circuit->reset(1U);
  circuit->if_(qc::X, 0U, 0U);
  circuit->measure(0U, 1U);

  CircuitSimulator simulator(std::move(circuit), 17U);
  const auto counts = simulator.simulate(8U);

  EXPECT_EQ(counts, (std::map<std::string, std::size_t>{{"11", 8U}}));
  EXPECT_EQ(simulator.additionalStatistics().at("single_shots"), "8");
  const auto amplitudes = simulator.getCurrentDD().getVector();
  ASSERT_EQ(amplitudes.size(), 4U);
  EXPECT_NEAR(std::abs(amplitudes.at(1U)), 1., 1e-12);
  const auto& roots = simulator.dd->getRootSet<dd::vNode>();
  ASSERT_EQ(roots.size(), 1U);
  EXPECT_EQ(roots.at(simulator.getCurrentDD()), 1U);
}

TEST(CircuitExecutionTest, ZeroShotsPreserveExecutionSemantics) {
  auto staticCircuit = std::make_unique<qc::QuantumComputation>(1U);
  staticCircuit->x(0U);
  CircuitSimulator staticSimulator(std::move(staticCircuit), 0U);

  EXPECT_TRUE(staticSimulator.simulate(0U).empty());
  EXPECT_EQ(staticSimulator.additionalStatistics().at("single_shots"), "1");
  EXPECT_NEAR(std::abs(staticSimulator.getCurrentDD().getVector().at(1U)), 1.,
              1e-12);

  auto dynamicCircuit = std::make_unique<qc::QuantumComputation>(1U, 1U);
  dynamicCircuit->measure(0U, 0U);
  dynamicCircuit->x(0U);
  CircuitSimulator dynamicSimulator(std::move(dynamicCircuit), 0U);

  EXPECT_TRUE(dynamicSimulator.simulate(0U).empty());
  EXPECT_EQ(dynamicSimulator.additionalStatistics().at("single_shots"), "0");
  EXPECT_NEAR(std::abs(dynamicSimulator.getCurrentDD().getVector().at(0U)), 1.,
              1e-12);
}

TEST(CircuitExecutionTest, RepeatedCallsReuseRngWithoutLeakingRoots) {
  const auto makeCircuit = [] {
    auto circuit = std::make_unique<qc::QuantumComputation>(1U);
    circuit->h(0U);
    return circuit;
  };

  CircuitSimulator splitSimulator(makeCircuit(), 0U);
  auto splitCounts = splitSimulator.simulate(17U);
  for (const auto& [state, count] : splitSimulator.simulate(23U)) {
    splitCounts[state] += count;
  }

  CircuitSimulator combinedSimulator(makeCircuit(), 0U);
  const auto combinedCounts = combinedSimulator.simulate(40U);

  EXPECT_EQ(splitCounts, combinedCounts);
  EXPECT_EQ(splitSimulator.additionalStatistics().at("single_shots"), "2");
  EXPECT_EQ(combinedSimulator.additionalStatistics().at("single_shots"), "1");

  const auto& roots = splitSimulator.dd->getRootSet<dd::vNode>();
  ASSERT_EQ(roots.size(), 1U);
  EXPECT_EQ(roots.at(splitSimulator.getCurrentDD()), 1U);
}

TEST(CircuitExecutionTest, DynamicRepeatedCallsReuseRng) {
  const auto makeCircuit = [] {
    auto circuit = std::make_unique<qc::QuantumComputation>(1U, 1U);
    circuit->h(0U);
    circuit->measure(0U, 0U);
    circuit->x(0U);
    return circuit;
  };

  CircuitSimulator splitSimulator(makeCircuit(), 0U);
  auto splitCounts = splitSimulator.simulate(17U);
  for (const auto& [state, count] : splitSimulator.simulate(23U)) {
    splitCounts[state] += count;
  }

  CircuitSimulator combinedSimulator(makeCircuit(), 0U);
  const auto combinedCounts = combinedSimulator.simulate(40U);

  EXPECT_EQ(splitCounts, combinedCounts);
  EXPECT_EQ(splitSimulator.additionalStatistics().at("single_shots"), "40");
  EXPECT_EQ(combinedSimulator.additionalStatistics().at("single_shots"), "40");
}

TEST(CircuitExecutionTest, FailedExecutionKeepsPreviousStateAndCanRetry) {
  auto circuit = std::make_unique<qc::QuantumComputation>(1U, 1U);
  circuit->x(0U);
  auto* mutableCircuit = circuit.get();
  CircuitSimulator simulator(std::move(circuit), 17U);

  EXPECT_EQ(simulator.simulate(1U),
            (std::map<std::string, std::size_t>{{"1", 1U}}));
  const auto previousState = simulator.getCurrentDD();

  mutableCircuit->measure(0U, 0U);
  auto& measurement = dynamic_cast<qc::NonUnitaryOperation&>(
      *mutableCircuit->at(mutableCircuit->getNops() - 1U));
  measurement.getClassics().clear();
  EXPECT_THROW(static_cast<void>(simulator.simulate(1U)),
               std::invalid_argument);

  EXPECT_EQ(simulator.getCurrentDD(), previousState);
  const auto& rootsAfterFailure = simulator.dd->getRootSet<dd::vNode>();
  ASSERT_EQ(rootsAfterFailure.size(), 1U);
  EXPECT_EQ(rootsAfterFailure.at(previousState), 1U);

  measurement.getClassics().emplace_back(0U);
  EXPECT_EQ(simulator.simulate(1U),
            (std::map<std::string, std::size_t>{{"1", 1U}}));
  const auto& rootsAfterRetry = simulator.dd->getRootSet<dd::vNode>();
  ASSERT_EQ(rootsAfterRetry.size(), 1U);
  EXPECT_EQ(rootsAfterRetry.at(simulator.getCurrentDD()), 1U);
}

TEST(CircuitExecutionTest, DerivedSimulatorKeepsExecutionHooks) {
  auto circuit = std::make_unique<qc::QuantumComputation>(1U, 2U);
  circuit->x(0U);
  circuit->measure(0U, 0U);
  circuit->measure(0U, 1U);
  HookedCircuitSimulator simulator(std::move(circuit), 17U);

  EXPECT_EQ(simulator.simulate(1U),
            (std::map<std::string, std::size_t>{{"11", 1U}}));
  EXPECT_TRUE(simulator.operationHookWasCalled());
}

TEST(CircuitExecutionTest, ApproximationKeepsHookExecutionLoop) {
  auto circuit = std::make_unique<qc::QuantumComputation>(3U);
  circuit->h(0U);
  circuit->h(1U);
  circuit->mcx(qc::Controls{qc::Control{0U}, qc::Control{1U}}, 2U);
  circuit->i(1U);
  circuit->i(1U);

  CircuitSimulator simulator(
      std::move(circuit),
      ApproximationInfo(0.3, 1U, ApproximationInfo::FidelityDriven), 17U);
  simulator.simulate(1U);

  EXPECT_EQ(simulator.getActiveNodeCount(), 3U);
  EXPECT_EQ(simulator.additionalStatistics().at("approximation_runs"), "1");
  EXPECT_DOUBLE_EQ(
      std::stod(simulator.additionalStatistics().at("final_fidelity")), 0.5);
}

TEST(CircuitExecutionTest, DeterministicNoiseKeepsDensityExecutionLoop) {
  auto circuit = std::make_unique<qc::QuantumComputation>(2U, 2U);
  circuit->x(0U);
  circuit->x(1U);
  circuit->reset(0U);
  circuit->measure(0U, 0U);
  circuit->measure(1U, 1U);

  DeterministicNoiseSimulator simulator(std::move(circuit), std::string("A"),
                                        0., 0., 1.);
  const auto counts = simulator.simulate(16U);

  EXPECT_EQ(counts, (std::map<std::string, std::size_t>{{"10", 16U}}));
  EXPECT_EQ(simulator.additionalStatistics().at("single_shots"), "16");

  EXPECT_EQ(simulator.simulate(16U), counts);
  EXPECT_EQ(simulator.additionalStatistics().at("single_shots"), "32");
  const auto& roots = simulator.dd->getRootSet<dd::mNode>();
  ASSERT_EQ(roots.size(), 1U);
  EXPECT_EQ(roots.begin()->second, 1U);
}

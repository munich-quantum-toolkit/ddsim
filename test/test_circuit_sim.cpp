/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "CircuitGenerators.hpp"
#include "CircuitSimulator.hpp"
#include "DDMinimizer.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Package.hpp"
#include "dd/Simulation.hpp"
#include "dd/StateGeneration.hpp"
#include "ir/Definitions.hpp"
#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/IfElseOperation.hpp"
#include "ir/operations/NonUnitaryOperation.hpp"
#include "ir/operations/OpType.hpp"
#include "ir/operations/StandardOperation.hpp"

#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>

TEST(CircuitSimTest, SingleOneQubitGateOnTwoQubitCircuit) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2);
  quantumComputation->x(0);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));

  ASSERT_EQ(ddsim.getNumberOfOps(), 1);

  ddsim.simulate(1);

  const auto m = ddsim.measureAll(false);

  ASSERT_EQ("01", m);
}

TEST(CircuitSimTest, SingleOneQubitSingleShot) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2);
  quantumComputation->h(0);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));

  ASSERT_EQ(ddsim.getNumberOfOps(), 1);

  ddsim.simulate(10);
  ASSERT_EQ("1", ddsim.additionalStatistics().at("single_shots"));
}

TEST(CircuitSimTest, SingleOneQubitSingleShot2) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->h(0);
  quantumComputation->measure(0, 0);
  std::cout << *quantumComputation << "\n";
  CircuitSimulator ddsim(std::move(quantumComputation), ApproximationInfo(),
                         1337);

  ASSERT_EQ(ddsim.getNumberOfOps(), 2);

  ddsim.simulate(10);
  ASSERT_EQ("1", ddsim.additionalStatistics().at("single_shots"));
}

TEST(CircuitSimTest, SingleOneQubitMultiShots) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->h(0);
  quantumComputation->measure(0, 0);
  quantumComputation->h(0);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));

  ASSERT_EQ(ddsim.getNumberOfOps(), 3);

  ddsim.simulate(10);
  ASSERT_EQ("10", ddsim.additionalStatistics().at("single_shots"));
}

TEST(CircuitSimTest, BarrierStatement) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(1);
  quantumComputation->h(0);
  quantumComputation->barrier(0);
  quantumComputation->h(0);
  CircuitSimulator ddsim(std::move(quantumComputation), ApproximationInfo());

  ASSERT_EQ(ddsim.getNumberOfOps(), 3);

  ddsim.simulate(10);
  ASSERT_EQ("1", ddsim.additionalStatistics().at("single_shots"));
}

TEST(CircuitSimTest, IfElseOpBitEq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(std::make_unique<qc::StandardOperation>(1U, qc::X),
                             std::make_unique<qc::StandardOperation>(1U, qc::I),
                             0U, true, qc::Eq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("11", m);
}

TEST(CircuitSimTest, IfElseOpBitNeq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(std::make_unique<qc::StandardOperation>(1U, qc::X),
                             std::make_unique<qc::StandardOperation>(1U, qc::I),
                             0U, true, qc::Neq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("01", m);
}

TEST(CircuitSimTest, IfElseOpRegisterEq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Eq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("11", m);
}

TEST(CircuitSimTest, IfElseOpRegisterNeq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Neq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("01", m);
}

TEST(CircuitSimTest, IfElseOpRegisterLt) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Lt);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("01", m);
}

TEST(CircuitSimTest, IfElseOpRegisterLeq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Leq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("11", m);
}

TEST(CircuitSimTest, IfElseOpRegisterGt) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Gt);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("01", m);
}

TEST(CircuitSimTest, IfElseOpRegisterGeq) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2, 2);
  quantumComputation->x(0);
  quantumComputation->measure(0, 0);
  quantumComputation->ifElse(
      std::make_unique<qc::StandardOperation>(1U, qc::X),
      std::make_unique<qc::StandardOperation>(1U, qc::I),
      quantumComputation->getClassicalRegisters().at("c"), 1U, qc::Geq);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  auto m = ddsim.measureAll(false);

  ASSERT_EQ("11", m);
}

TEST(CircuitSimTest, DestructiveMeasurementAll) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(2);
  quantumComputation->h(0);
  quantumComputation->h(1);
  CircuitSimulator ddsim(std::move(quantumComputation), 42);
  ddsim.simulate(1);

  const auto vBefore = ddsim.getCurrentDD().getVector();
  ASSERT_EQ(vBefore[0], vBefore[1]);
  ASSERT_EQ(vBefore[0], vBefore[2]);
  ASSERT_EQ(vBefore[0], vBefore[3]);

  const std::string m = ddsim.measureAll(true);
  const auto vAfter = ddsim.getCurrentDD().getVector();
  const std::size_t i = std::stoul(m, nullptr, 2);

  ASSERT_EQ(vAfter[i].real(), 1.0);
  ASSERT_EQ(vAfter[i].imag(), 0.0);
}

TEST(CircuitSimTest, ApproximateByFidelity) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(0);
  quantumComputation->h(1);
  quantumComputation->mcx(qc::Controls{qc::Control{0}, qc::Control{1}}, 2);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  std::cout << ddsim.getActiveNodeCount() << "\n";
  ddsim.simulate(1);
  ASSERT_EQ(ddsim.getActiveNodeCount(), 6);

  double const resultingFidelity =
      ddsim.approximateByFidelity(0.3, false, true, true);

  ASSERT_EQ(ddsim.getActiveNodeCount(), 3);
  ASSERT_DOUBLE_EQ(resultingFidelity, 0.5); // equal up to 4 ULP
}

TEST(CircuitSimTest, ApproximateBySampling) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(0);
  quantumComputation->h(1);
  quantumComputation->mcx(qc::Controls{qc::Control{0}, qc::Control{1}}, 2);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  ASSERT_EQ(ddsim.getActiveNodeCount(), 6);

  double const resultingFidelity =
      ddsim.approximateBySampling(1, 0, true, true);

  ASSERT_EQ(ddsim.getActiveNodeCount(), 3);
  ASSERT_LE(resultingFidelity, 0.75); // the least contributing path has .25
}

TEST(CircuitSimTest, ApproximationByFidelityInSimulator) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(0);
  quantumComputation->h(1);
  quantumComputation->mcx(qc::Controls{qc::Control{0}, qc::Control{1}}, 2);

  quantumComputation->i(1); // some dummy operations
  quantumComputation->i(1);

  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(0.3, 1, ApproximationInfo::FidelityDriven));
  ddsim.simulate(1);

  ASSERT_EQ(ddsim.getActiveNodeCount(), 3);
  ASSERT_DOUBLE_EQ(std::stod(ddsim.additionalStatistics()["final_fidelity"]),
                   0.5);
}

TEST(CircuitSimTest, TestingProperties) {
  auto quantumComputation = std::make_unique<qc::QuantumComputation>(3);
  quantumComputation->h(0);
  quantumComputation->h(1);
  quantumComputation->mcx(qc::Controls{qc::Control{0}, qc::Control{1}}, 2);
  CircuitSimulator ddsim(
      std::move(quantumComputation),
      ApproximationInfo(1, 1, ApproximationInfo::FidelityDriven), 1);
  ddsim.simulate(1);

  EXPECT_EQ(ddsim.getActiveNodeCount(), 6);
  EXPECT_EQ(ddsim.getMatrixActiveNodeCount(), 0);
  EXPECT_EQ(ddsim.countNodesFromRoot(), 7);
  EXPECT_EQ(ddsim.getSeed(), "1");
  EXPECT_EQ(ddsim.additionalStatistics().at("approximation_runs"), "0");
}

TEST(CircuitSimTest, ApproximationTest) {
  // the following creates a state where the first qubit has a <2% probability
  // of being 1
  auto qc = std::make_unique<qc::QuantumComputation>(2);
  qc->h(0);
  qc->cry(qc::PI / 8, 0, 1);

  // approximating the state with fidelity 0.98 should allow to eliminate the
  // 1-successor of the first qubit
  CircuitSimulator ddsim(
      std::move(qc),
      ApproximationInfo(0.98, 2, ApproximationInfo::FidelityDriven));
  ddsim.simulate(4096);
  const auto vec = ddsim.getCurrentDD().getVector();
  EXPECT_EQ(abs(vec[2]), 0);
  EXPECT_EQ(abs(vec[3]), 0);
}

TEST(CircuitSimTest, expectationValueLocalOperators) {
  const std::size_t maxQubits = 3;
  for (std::size_t nrQubits = 1; nrQubits < maxQubits; ++nrQubits) {
    auto qc = std::make_unique<qc::QuantumComputation>(nrQubits);
    CircuitSimulator ddsim(std::move(qc));
    for (qc::Qubit site = 0; site < nrQubits; ++site) {
      auto xObservable = qc::QuantumComputation(nrQubits);
      xObservable.x(site);
      EXPECT_EQ(ddsim.expectationValue(xObservable), 0);
      auto zObservable = qc::QuantumComputation(nrQubits);
      zObservable.z(site);
      EXPECT_EQ(ddsim.expectationValue(zObservable), 1);
      auto hObservable = qc::QuantumComputation(nrQubits);
      hObservable.h(site);
      EXPECT_EQ(ddsim.expectationValue(hObservable), dd::SQRT2_2);
    }
  }
}

TEST(CircuitSimTest, expectationValueGlobalOperators) {
  const std::size_t maxQubits = 3;
  for (std::size_t nrQubits = 1; nrQubits < maxQubits; ++nrQubits) {
    auto qc = std::make_unique<qc::QuantumComputation>(nrQubits);
    CircuitSimulator ddsim(std::move(qc));
    auto xObservable = qc::QuantumComputation(nrQubits);
    auto zObservable = qc::QuantumComputation(nrQubits);
    auto hObservable = qc::QuantumComputation(nrQubits);
    for (qc::Qubit site = 0; site < nrQubits; ++site) {
      xObservable.x(site);
      zObservable.z(site);
      hObservable.h(site);
    }
    EXPECT_EQ(ddsim.expectationValue(xObservable), 0);
    EXPECT_EQ(ddsim.expectationValue(zObservable), 1);
    EXPECT_EQ(ddsim.expectationValue(hObservable),
              std::pow(dd::SQRT2_2, nrQubits));
  }
}

TEST(CircuitSimTest, ToleranceTest) {
  // A small test to make sure that setting and getting the tolerance works
  auto qc = std::make_unique<qc::QuantumComputation>(2);
  CircuitSimulator ddsim(std::move(qc));
  const auto tolerance = ddsim.getTolerance();
  constexpr auto newTolerance = 0.1;
  ddsim.setTolerance(newTolerance);
  EXPECT_EQ(ddsim.getTolerance(), newTolerance);
  ddsim.setTolerance(tolerance);
  EXPECT_EQ(ddsim.getTolerance(), tolerance);
}

TEST(CircuitSimTest, BernsteinVaziraniDynamicTest) {
  constexpr std::size_t n = 3;
  const auto* const expectedString = "110";
  const ddsim::detail::BernsteinVaziraniBitString expected{expectedString};
  auto qc = std::make_unique<qc::QuantumComputation>(
      ddsim::detail::createIterativeBernsteinVazirani(expected, n));
  EXPECT_EQ(qc->getName(), "iterative_bv_110");
  const auto circSim = std::make_unique<CircuitSimulator>(std::move(qc), 23);
  const auto result = circSim->simulate(1024U);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.at(expectedString), 1024);
}

TEST(CircuitSimTest, QPEDynamicTest) {
  constexpr std::size_t n = 3;
  auto qc = std::make_unique<qc::QuantumComputation>(
      ddsim::detail::createIterativeQPE(n));
  const auto circSim = std::make_unique<CircuitSimulator>(std::move(qc), 23);
  const auto result = circSim->simulate(1024U);
  EXPECT_GE(result.size(), 1);
}

TEST(CircuitSimTest, QFTDynamicTest) {
  constexpr std::size_t n = 3;
  auto qc = std::make_unique<qc::QuantumComputation>(
      ddsim::detail::createIterativeQFT(n));
  const auto circSim = std::make_unique<CircuitSimulator>(std::move(qc), 23);
  const auto result = circSim->simulate(1024U);
  EXPECT_GE(result.size(), 1);
}

TEST(CircuitGeneratorTest, GHZState) {
  auto qc = std::make_unique<qc::QuantumComputation>(
      ddsim::detail::createGHZState(3));
  CircuitSimulator ddsim(std::move(qc));
  ddsim.simulate(1);

  const auto state = ddsim.getCurrentDD().getVector();
  ASSERT_EQ(state.size(), 8);
  EXPECT_NEAR(std::norm(state[0]), 0.5, 1e-10);
  EXPECT_NEAR(std::norm(state[7]), 0.5, 1e-10);
  for (std::size_t i = 1; i < state.size() - 1; ++i) {
    EXPECT_NEAR(std::norm(state[i]), 0., 1e-10);
  }
}

TEST(CircuitGeneratorTest, QFTState) {
  constexpr std::size_t n = 3;
  auto qc = std::make_unique<qc::QuantumComputation>(
      ddsim::detail::createQFT(n, false));
  CircuitSimulator ddsim(std::move(qc));
  ddsim.simulate(1);

  const auto state = ddsim.getCurrentDD().getVector();
  ASSERT_EQ(state.size(), 1U << n);
  for (const auto& amplitude : state) {
    EXPECT_NEAR(std::norm(amplitude), 1. / static_cast<double>(state.size()),
                1e-10);
  }
}

TEST(CircuitGeneratorTest, QFTMeasurements) {
  constexpr qc::Qubit n = 3;
  const auto qc = ddsim::detail::createQFT(n);
  ASSERT_GE(qc.getNops(), n);
  for (qc::Qubit i = 0; i < n; ++i) {
    const auto& operation = qc.at(qc.getNops() - n + i);
    ASSERT_EQ(operation->getType(), qc::Measure);
    const auto* measurement =
        dynamic_cast<const qc::NonUnitaryOperation*>(operation.get());
    ASSERT_NE(measurement, nullptr);
    ASSERT_EQ(measurement->getTargets().size(), 1);
    EXPECT_EQ(measurement->getTargets().front(), i);
    ASSERT_EQ(measurement->getClassics().size(), 1);
    EXPECT_EQ(measurement->getClassics().front(), n - 1 - i);
  }
}

TEST(CircuitGeneratorTest, SeededGroverIsDeterministic) {
  const auto first = ddsim::detail::createGrover(5, std::size_t{23});
  const auto second = ddsim::detail::createGrover(5, std::size_t{23});
  EXPECT_EQ(first.getName(), second.getName());
  EXPECT_EQ(first, second);
}

TEST(CircuitGeneratorTest, ZeroSeedSelectsRandomGroverTargets) {
  auto names = std::set<std::string>{};
  for (std::size_t i = 0; i < 8; ++i) {
    names.emplace(ddsim::detail::createGrover(8, std::size_t{0}).getName());
  }
  EXPECT_GT(names.size(), 1);
}

TEST(CircuitGeneratorTest, ZeroSeedSelectsRandomQPEPhases) {
  auto names = std::set<std::string>{};
  for (std::size_t i = 0; i < 8; ++i) {
    names.emplace(
        ddsim::detail::createIterativeQPE(8, true, std::size_t{0}).getName());
  }
  EXPECT_GT(names.size(), 1);
}

TEST(CircuitSimTest, GetVectorBeforeSimulate) {
  auto qc = std::make_unique<qc::QuantumComputation>(1);
  const CircuitSimulator ddsim(std::move(qc));
  const auto vec = ddsim.getCurrentDD().getVector();
  EXPECT_EQ(vec[0], 1.);
}

TEST(CircuitSimTest, TracksInputAndOutputLayouts) {
  auto circuit = std::make_unique<qc::QuantumComputation>(3);
  circuit->initialLayout = {{0, 2}, {1, 0}, {2, 1}};
  circuit->outputPermutation = {{0, 1}, {1, 2}, {2, 0}};
  circuit->x(0);
  CircuitSimulator sim(std::move(circuit), 42U);

  EXPECT_EQ(sim.simulate(16).at("010"), 16);
  EXPECT_EQ(sim.measureAll(false), "010");
  EXPECT_NEAR(std::norm(sim.getCurrentDD().getVector().at(2)), 1., 1e-12);
  qc::QuantumComputation observable(3);
  observable.z(1);
  EXPECT_NEAR(sim.expectationValue(observable), -1., 1e-12);
}

TEST(CircuitSimTest, TracksFinalMeasurementsWithOutputLayout) {
  auto circuit = std::make_unique<qc::QuantumComputation>(3, 5);
  circuit->initialLayout = {{0, 2}, {1, 0}, {2, 1}};
  circuit->outputPermutation = {{0, 1}, {1, 2}, {2, 0}};
  circuit->x(0);
  circuit->measure(0, 4);
  circuit->measure(1, 1);
  circuit->measure(2, 0);
  CircuitSimulator sim(std::move(circuit), 42U);

  EXPECT_EQ(sim.simulate(16).at("10000"), 16);
  EXPECT_NEAR(std::norm(sim.getCurrentDD().getVector().at(2)), 1., 1e-12);
}

TEST(CircuitSimTest, ResetsVirtualSwapPermutationBetweenRuns) {
  auto circuit = std::make_unique<qc::QuantumComputation>(3);
  circuit->initialLayout = {{0, 1}, {1, 2}, {2, 0}};
  circuit->x(0);
  circuit->swap(0, 2);
  circuit->cx(2, 1);
  CircuitSimulator sim(std::move(circuit), 42U);

  for (int run = 0; run < 2; ++run) {
    EXPECT_EQ(sim.simulate(16).at("110"), 16);
    EXPECT_NEAR(std::norm(sim.getCurrentDD().getVector().at(6)), 1., 1e-12);
  }
}

TEST(CircuitSimTest, OptimizedStatesMatchCoreForFlatAndCompoundCircuits) {
  for (const bool grouped : {false, true}) {
    qc::QuantumComputation circuit(4);
    circuit.h(0);
    circuit.ry(0.3, 1);
    circuit.cx(0, 1);
    circuit.cx(1, 3);
    circuit.cx(3, 2);
    circuit.t(3);
    circuit.swap(0, 2);
    circuit.cz(1, 2);
    circuit.outputPermutation = {{0, 2}, {1, 0}, {2, 3}, {3, 1}};
    if (grouped) {
      auto compound = circuit.asCompoundOperation();
      circuit.emplace_back(std::move(compound));
    }
    dd::Package package(4);
    const auto expected =
        dd::simulate(circuit, dd::makeZeroState(4, package), package)
            .getVector();
    const auto initialLayout = circuit.initialLayout;
    ddsim::DDMinimizer::optimizeInputPermutation(circuit);
    ASSERT_NE(circuit.initialLayout, initialLayout);
    CircuitSimulator sim(std::make_unique<qc::QuantumComputation>(circuit),
                         42U);

    EXPECT_TRUE(sim.simulate(0).empty());
    const auto actual = sim.getCurrentDD().getVector();
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); ++i) {
      EXPECT_NEAR(std::abs(actual.at(i) - expected.at(i)), 0., 1e-12);
    }
  }
}

TEST(CircuitSimTest, SimulatesOptimizedSparseLayouts) {
  for (const bool measured : {false, true}) {
    auto circuit = std::make_unique<qc::QuantumComputation>();
    circuit->addQubit(0, 0, 0);
    circuit->addQubit(1, 2, 1);
    circuit->addQubit(2, 5, 2);
    circuit->addQubit(3, 7, 3);
    circuit->x(5);
    circuit->cx(0, 2);
    circuit->cx(2, 5);
    circuit->cx(5, 7);
    if (measured) {
      circuit->addClassicalRegister(4);
      circuit->measure(0, 3);
      circuit->measure(2, 1);
      circuit->measure(5, 0);
      circuit->measure(7, 2);
    }
    ddsim::DDMinimizer::optimizeInputPermutation(*circuit);
    CircuitSimulator sim(std::move(circuit), 42U);

    EXPECT_EQ(sim.simulate(16).at(measured ? "0101" : "1100"), 16);
    EXPECT_NEAR(std::norm(sim.getCurrentDD().getVector().at(12)), 1., 1e-12);
  }
}

TEST(CircuitSimTest, TracksDynamicMeasurementsResetsAndBranches) {
  for (const bool takeThen : {false, true}) {
    auto circuit = std::make_unique<qc::QuantumComputation>(3, 3);
    circuit->initialLayout = {{0, 2}, {1, 0}, {2, 1}};
    if (takeThen) {
      circuit->x(0);
    }
    circuit->swap(0, 1);
    circuit->measure(1, 0);
    circuit->reset(1);
    circuit->ifElse(std::make_unique<qc::StandardOperation>(2U, qc::X), nullptr,
                    0U, true, qc::Eq);
    circuit->measure(2, 1);
    circuit->ifElse(
        std::make_unique<qc::StandardOperation>(qc::Targets{0, 2}, qc::SWAP),
        std::make_unique<qc::StandardOperation>(0U, qc::X), 0U, true, qc::Eq);
    circuit->measure(0, 2);
    CircuitSimulator sim(std::move(circuit), 42U);

    for (int run = 0; run < 2; ++run) {
      EXPECT_EQ(sim.simulate(16).at(takeThen ? "111" : "100"), 16);
      EXPECT_NEAR(std::norm(sim.getCurrentDD().getVector().at(1)), 1., 1e-12);
    }
  }
}

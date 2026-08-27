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
#include "HybridSchrodingerFeynmanSimulator.hpp"
#include "PathSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "UnitarySimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <nanobind/nanobind.h>
#include <nanobind/stl/complex.h>  // NOLINT(misc-include-cleaner)
#include <nanobind/stl/list.h>     // NOLINT(misc-include-cleaner)
#include <nanobind/stl/map.h>      // NOLINT(misc-include-cleaner)
#include <nanobind/stl/optional.h> // NOLINT(misc-include-cleaner)
#include <nanobind/stl/pair.h>     // NOLINT(misc-include-cleaner)
#include <nanobind/stl/string.h>   // NOLINT(misc-include-cleaner)
#include <nanobind/stl/vector.h>   // NOLINT(misc-include-cleaner)
#include <optional>
#include <string>
#include <utility>

namespace nb = nanobind;
using namespace nb::literals;

namespace {

template <class Sim>
nb::class_<Sim> createSimulator(nb::module_ m, const std::string& name) {
  auto sim = nb::class_<Sim>(std::move(m), name.c_str());
  sim.def("get_number_of_qubits", &Sim::getNumberOfQubits,
          "Get the number of qubits")
      .def("get_name", &Sim::getName, "Get the name of the simulator")
      .def("statistics", &Sim::additionalStatistics,
           "Get additional statistics provided by the simulator")
      .def("get_active_vector_node_count", &Sim::getActiveNodeCount,
           "Get the number of active vector nodes, i.e., the number of vector "
           "DD nodes in the unique table with a non-zero reference count.")
      .def("get_active_matrix_node_count", &Sim::getMatrixActiveNodeCount,
           "Get the number of active matrix nodes, i.e., the number of matrix "
           "DD nodes in the unique table with a non-zero reference count.")
      .def("get_tolerance", &Sim::getTolerance,
           "Get the tolerance for the DD package.")
      .def("set_tolerance", &Sim::setTolerance, "tol"_a,
           "Set the tolerance for the DD package.");

  if constexpr (std::is_same_v<Sim, UnitarySimulator>) {
    sim.def("construct", &Sim::construct,
            "Construct the DD representing the unitary matrix of the circuit.");
  } else {
    sim.def("simulate", &Sim::simulate, "shots"_a,
            "Simulate the circuit and return the result as a dictionary of "
            "counts.");
    sim.def("get_constructed_dd", &Sim::getCurrentDD,
            "Get the vector DD resulting from the simulation.");
  }
  return sim;
}

} // namespace

// NOLINTNEXTLINE(performance-unnecessary-value-param)
NB_MODULE(MQT_DDSIM_MODULE_NAME, m) {
  nb::module_::import_("mqt.core.dd");

  // Circuit Simulator
  auto circuitSimulator =
      createSimulator<CircuitSimulator>(m, "CircuitSimulator");
  circuitSimulator
      .def(
          "__init__",
          [](CircuitSimulator* self, const qc::QuantumComputation& circ,
             const double stepFidelity, const unsigned int stepNumber,
             const std::string& approximationStrategy,
             const std::int64_t seed) {
            auto qc = std::make_unique<qc::QuantumComputation>(circ);
            const auto approx = ApproximationInfo{
                stepFidelity, stepNumber,
                ApproximationInfo::fromString(approximationStrategy)};
            if (seed < 0) {
              new (self) CircuitSimulator(std::move(qc), approx);
            } else {
              new (self) CircuitSimulator(std::move(qc), approx,
                                          static_cast<std::uint64_t>(seed));
            }
          },
          "circ"_a, "approximation_step_fidelity"_a = 1.,
          "approximation_steps"_a = 1, "approximation_strategy"_a = "fidelity",
          "seed"_a = -1)
      .def("expectation_value", &CircuitSimulator::expectationValue,
           "observable"_a,
           "Compute the expectation value for the given observable.");

  // Stoch simulator
  auto stochasticNoiseSimulator =
      createSimulator<StochasticNoiseSimulator>(m, "StochasticNoiseSimulator");
  stochasticNoiseSimulator.def(
      "__init__",
      [](StochasticNoiseSimulator* self, const qc::QuantumComputation& circ,
         const double stepFidelity, const unsigned int stepNumber,
         const std::string& approximationStrategy, const std::int64_t seed,
         const std::string& noiseEffects, const double noiseProbability,
         std::optional<double> ampDampingProb,
         const double multiQubitGateFactor) {
        auto qc = std::make_unique<qc::QuantumComputation>(circ);
        const auto approx = ApproximationInfo{
            stepFidelity, stepNumber,
            ApproximationInfo::fromString(approximationStrategy)};
        if (seed < 0) {
          new (self) StochasticNoiseSimulator(
              std::move(qc), approx, noiseEffects, noiseProbability,
              ampDampingProb, multiQubitGateFactor);
        } else {
          new (self) StochasticNoiseSimulator(
              std::move(qc), approx, static_cast<std::size_t>(seed),
              noiseEffects, noiseProbability, ampDampingProb,
              multiQubitGateFactor);
        }
      },
      "circ"_a, "approximation_step_fidelity"_a = 1.,
      "approximation_steps"_a = 1, "approximation_strategy"_a = "fidelity",
      "seed"_a = -1, "noise_effects"_a = "APD", "noise_probability"_a = 0.01,
      "amp_damping_probability"_a = 0.02, "multi_qubit_gate_factor"_a = 2);

  // Deterministic simulator
  auto deterministicNoiseSimulator =
      createSimulator<DeterministicNoiseSimulator>(
          m, "DeterministicNoiseSimulator");
  deterministicNoiseSimulator.def(
      "__init__",
      [](DeterministicNoiseSimulator* self, const qc::QuantumComputation& circ,
         const double stepFidelity, const unsigned int stepNumber,
         const std::string& approximationStrategy, const std::int64_t seed,
         const std::string& noiseEffects, const double noiseProbability,
         std::optional<double> ampDampingProb,
         const double multiQubitGateFactor) {
        auto qc = std::make_unique<qc::QuantumComputation>(circ);
        const auto approx = ApproximationInfo{
            stepFidelity, stepNumber,
            ApproximationInfo::fromString(approximationStrategy)};
        if (seed < 0) {
          new (self) DeterministicNoiseSimulator(
              std::move(qc), approx, noiseEffects, noiseProbability,
              ampDampingProb, multiQubitGateFactor);
        } else {
          new (self) DeterministicNoiseSimulator(
              std::move(qc), approx, static_cast<std::size_t>(seed),
              noiseEffects, noiseProbability, ampDampingProb,
              multiQubitGateFactor);
        }
      },
      "circ"_a, "approximation_step_fidelity"_a = 1.,
      "approximation_steps"_a = 1, "approximation_strategy"_a = "fidelity",
      "seed"_a = -1, "noise_effects"_a = "APD", "noise_probability"_a = 0.01,
      "amp_damping_probability"_a = 0.02, "multi_qubit_gate_factor"_a = 2);

  // Hybrid Schrödinger-Feynman Simulator
  nb::enum_<HybridSchrodingerFeynmanSimulator::Mode>(
      m, "HybridSimulatorMode",
      R"pb(Enumeration of modes for the :class:`~HybridSimulator`.)pb")
      .value("DD", HybridSchrodingerFeynmanSimulator::Mode::DD)
      .value("amplitude", HybridSchrodingerFeynmanSimulator::Mode::Amplitude);

  auto hsfSimulator =
      createSimulator<HybridSchrodingerFeynmanSimulator>(m, "HybridSimulator");
  hsfSimulator
      .def(
          "__init__",
          [](HybridSchrodingerFeynmanSimulator* self,
             const qc::QuantumComputation& circ, const double stepFidelity,
             const unsigned int stepNumber,
             const std::string& approximationStrategy, const std::int64_t seed,
             HybridSchrodingerFeynmanSimulator::Mode mode,
             const std::size_t nthreads) {
            auto qc = std::make_unique<qc::QuantumComputation>(circ);
            const auto approx = ApproximationInfo{
                stepFidelity, stepNumber,
                ApproximationInfo::fromString(approximationStrategy)};
            if (seed < 0) {
              new (self) HybridSchrodingerFeynmanSimulator(
                  std::move(qc), approx, mode, nthreads);
            } else {
              new (self) HybridSchrodingerFeynmanSimulator(
                  std::move(qc), approx, static_cast<std::uint64_t>(seed), mode,
                  nthreads);
            }
          },
          "circ"_a, "approximation_step_fidelity"_a = 1.,
          "approximation_steps"_a = 1, "approximation_strategy"_a = "fidelity",
          "seed"_a = -1,
          "mode"_a = HybridSchrodingerFeynmanSimulator::Mode::Amplitude,
          "nthreads"_a = 2)
      .def("get_mode", &HybridSchrodingerFeynmanSimulator::getMode,
           "Get the mode of the hybrid simulator.")
      .def("get_final_amplitudes",
           &HybridSchrodingerFeynmanSimulator::getVectorFromHybridSimulation,
           "Get the final amplitudes from the hybrid simulation.");

  // Path Simulator
  nb::enum_<PathSimulator::Configuration::Mode>(
      m, "PathSimulatorMode",
      "Enumeration of modes for the :class:`~PathSimulator`.")
      .value("sequential", PathSimulator::Configuration::Mode::Sequential)
      .value("pairwise_recursive",
             PathSimulator::Configuration::Mode::PairwiseRecursiveGrouping)
      .value("bracket", PathSimulator::Configuration::Mode::BracketGrouping)
      .value("alternating", PathSimulator::Configuration::Mode::Alternating)
      .value("gate_cost", PathSimulator::Configuration::Mode::GateCost);

  nb::class_<PathSimulator::Configuration>(
      m, "PathSimulatorConfiguration",
      R"pb(Configuration options for the :class:`~.PathSimulator`.)pb")
      .def(nb::init())
      .def_rw("mode", &PathSimulator::Configuration::mode,
              R"pb(The mode used for determining a simulation path.)pb")
      .def_rw("bracket_size", &PathSimulator::Configuration::bracketSize,
              R"pb(Size of the brackets one wants to combine.)pb")
      .def_rw("starting_point", &PathSimulator::Configuration::startingPoint,
              R"pb(Start of the alternating or gate_cost strategy.)pb")
      .def_rw(
          "gate_cost", &PathSimulator::Configuration::gateCost,
          R"pb(A list that contains the number of gates which are considered in each step.)pb")
      .def_rw("seed", &PathSimulator::Configuration::seed,
              R"pb(Seed for the simulator.)pb")
      .def(
          "json",
          [](const PathSimulator::Configuration& config) {
            const auto json = nb::module_::import_("json");
            const auto loads = json.attr("loads");
            const auto dict = loads(config.json().dump());
            return nb::cast<nb::typed<nb::dict, nb::str, nb::any>>(dict);
          },
          "Get the configuration as a JSON-style dictionary.")
      .def("__repr__", &PathSimulator::Configuration::toString);

  auto pathSimulator = createSimulator<PathSimulator>(m, "PathSimulator");
  pathSimulator
      .def(
          "__init__",
          [](PathSimulator* self, const qc::QuantumComputation& circ,
             const PathSimulator::Configuration& config) {
            auto qc = std::make_unique<qc::QuantumComputation>(circ);
            new (self) PathSimulator(std::move(qc), config);
          },
          "circ"_a, "config"_a = PathSimulator::Configuration())
      .def(
          "__init__",
          [](PathSimulator* self, const qc::QuantumComputation& circ,
             PathSimulator::Configuration::Mode mode,
             const std::size_t bracketSize, const std::size_t startingPoint,
             const std::list<std::size_t>& gateCost, const std::size_t seed) {
            auto qc = std::make_unique<qc::QuantumComputation>(circ);
            new (self) PathSimulator(std::move(qc), mode, bracketSize,
                                     startingPoint, gateCost, seed);
          },
          "circ"_a, "mode"_a = PathSimulator::Configuration::Mode::Sequential,
          "bracket_size"_a = 2, "starting_point"_a = 0,
          "gate_cost"_a = std::list<std::size_t>{}, "seed"_a = 0)
      .def("set_simulation_path",
           nb::overload_cast<const PathSimulator::SimulationPath::Components&,
                             bool>(&PathSimulator::setSimulationPath),
           "path"_a, "assume_correct_order"_a = false,
           R"pb(Set the simulation path.

Args:
    path: The components of the simulation path.
    assume_correct_order: Whether the provided path is assumed to be in the correct order. Defaults to False.)pb");

  // Unitary Simulator
  nb::enum_<UnitarySimulator::Mode>(
      m, "UnitarySimulatorMode",
      R"pb(Enumeration of modes for the :class:`~UnitarySimulator`.)pb")
      .value("recursive", UnitarySimulator::Mode::Recursive)
      .value("sequential", UnitarySimulator::Mode::Sequential);

  auto unitarySimulator =
      createSimulator<UnitarySimulator>(m, "UnitarySimulator");
  unitarySimulator
      .def(
          "__init__",
          [](UnitarySimulator* self, const qc::QuantumComputation& circ,
             const double stepFidelity, const unsigned int stepNumber,
             const std::string& approximationStrategy, const std::int64_t seed,
             UnitarySimulator::Mode mode) {
            auto qc = std::make_unique<qc::QuantumComputation>(circ);
            const auto approx = ApproximationInfo{
                stepFidelity, stepNumber,
                ApproximationInfo::fromString(approximationStrategy)};
            if (seed < 0) {
              new (self) UnitarySimulator(std::move(qc), approx, mode);
            } else {
              new (self)
                  UnitarySimulator(std::move(qc), approx,
                                   static_cast<std::uint64_t>(seed), mode);
            }
          },
          "circ"_a, "approximation_step_fidelity"_a = 1.,
          "approximation_steps"_a = 1, "approximation_strategy"_a = "fidelity",
          "seed"_a = -1, "mode"_a = UnitarySimulator::Mode::Recursive)
      .def("get_mode", &UnitarySimulator::getMode,
           "Get the mode of the unitary simulator.")
      .def("get_construction_time", &UnitarySimulator::getConstructionTime,
           "Get the time taken to construct the DD.")
      .def("get_final_node_count", &UnitarySimulator::getFinalNodeCount,
           "Get the final node count of the constructed DD.")
      .def("get_constructed_dd", &UnitarySimulator::getConstructedDD,
           "Get the constructed DD.");
}

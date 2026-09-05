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

#include "ir/Definitions.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/Control.hpp"
#include "ir/operations/IfElseOperation.hpp"
#include "ir/operations/OpType.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

namespace ddsim::detail {
namespace {
using qc::Control;
using qc::Controls;
using qc::fp;
using qc::QuantumComputation;
using qc::Qubit;

[[nodiscard]] auto createGenerator(const std::size_t seed) -> std::mt19937_64 {
  auto generator = std::mt19937_64{};
  if (seed != 0) {
    generator.seed(seed);
    return generator;
  }

  auto randomData =
      std::array<std::mt19937_64::result_type, std::mt19937_64::state_size>{};
  auto randomDevice = std::random_device{};
  std::ranges::generate(randomData,
                        [&randomDevice]() { return randomDevice(); });
  auto seedSequence = std::seed_seq(randomData.begin(), randomData.end());
  generator.seed(seedSequence);
  return generator;
}

auto appendGroverInitialization(QuantumComputation& circuit) -> void {
  const auto nDataQubits = static_cast<Qubit>(circuit.getNqubits() - 1);
  circuit.x(nDataQubits);
  for (Qubit i = 0; i < nDataQubits; ++i) {
    circuit.h(i);
  }
}

auto appendGroverOracle(QuantumComputation& circuit,
                        const GroverBitString& targetValue) -> void {
  const auto nDataQubits = static_cast<Qubit>(circuit.getNqubits() - 1);
  auto controls = Controls{};
  for (Qubit i = 0; i < nDataQubits; ++i) {
    controls.emplace(i, targetValue.test(i) ? Control::Type::Pos
                                            : Control::Type::Neg);
  }
  circuit.mcz(controls, nDataQubits);
}

auto appendGroverDiffusion(QuantumComputation& circuit) -> void {
  const auto nDataQubits = static_cast<Qubit>(circuit.getNqubits() - 1);
  for (Qubit i = 0; i < nDataQubits; ++i) {
    circuit.h(i);
  }
  for (Qubit i = 0; i < nDataQubits; ++i) {
    circuit.x(i);
  }
  auto controls = Controls{};
  for (Qubit i = 1; i < nDataQubits; ++i) {
    controls.emplace(i);
  }
  circuit.mcz(controls, 0);
  for (auto i = static_cast<std::make_signed_t<Qubit>>(nDataQubits - 1); i >= 0;
       --i) {
    circuit.x(static_cast<Qubit>(i));
  }
  for (auto i = static_cast<std::make_signed_t<Qubit>>(nDataQubits - 1); i >= 0;
       --i) {
    circuit.h(static_cast<Qubit>(i));
  }
}

[[nodiscard]] auto computeNumberOfGroverIterations(const Qubit nq)
    -> std::size_t {
  if (nq <= 2) {
    return 1;
  }
  if (nq % 2 == 1) {
    return static_cast<std::size_t>(std::round(
        qc::PI_4 * std::pow(2., (static_cast<double>(nq + 1) / 2.) - 1.) *
        std::numbers::sqrt2));
  }
  return static_cast<std::size_t>(
      std::round(qc::PI_4 * std::pow(2., static_cast<double>(nq) / 2.)));
}

[[nodiscard]] auto generateGroverTarget(const std::size_t nDataQubits,
                                        std::mt19937_64& generator)
    -> GroverBitString {
  auto targetValue = GroverBitString{};
  auto distribution = std::bernoulli_distribution{};
  for (std::size_t i = 0; i < nDataQubits; ++i) {
    if (distribution(generator)) {
      targetValue.set(i);
    }
  }
  return targetValue;
}

[[nodiscard]] auto getGroverName(const GroverBitString& targetValue,
                                 const Qubit nq) -> std::string {
  auto expected = targetValue.to_string();
  std::ranges::reverse(expected);
  while (expected.length() > nq) {
    expected.pop_back();
  }
  std::ranges::reverse(expected);
  return "grover_" + std::to_string(nq) + "_" + expected;
}

[[nodiscard]] auto
getBernsteinVaziraniBitString(const BernsteinVaziraniBitString& hiddenString,
                              const Qubit nq) -> std::string {
  auto expected = hiddenString.to_string();
  std::ranges::reverse(expected);
  while (expected.length() > nq) {
    expected.pop_back();
  }
  std::ranges::reverse(expected);
  return expected;
}

auto constructGroverCircuit(QuantumComputation& circuit, const Qubit nq,
                            const GroverBitString& targetValue) -> void {
  circuit.setName(getGroverName(targetValue, nq));
  circuit.addQubitRegister(nq, "q");
  circuit.addQubitRegister(1, "flag");
  circuit.addClassicalRegister(nq);

  appendGroverInitialization(circuit);
  const auto iterations = computeNumberOfGroverIterations(nq);
  for (std::size_t i = 0; i < iterations; ++i) {
    appendGroverOracle(circuit, targetValue);
    appendGroverDiffusion(circuit);
  }
  for (Qubit i = 0; i < nq; ++i) {
    circuit.measure(i, i);
  }
}

auto constructIterativeBernsteinVaziraniCircuit(
    QuantumComputation& circuit, const BernsteinVaziraniBitString& hiddenString,
    const Qubit nq) -> void {
  circuit.setName("iterative_bv_" +
                  getBernsteinVaziraniBitString(hiddenString, nq));
  circuit.addQubitRegister(1, "flag");
  circuit.addQubitRegister(1, "q");
  circuit.addClassicalRegister(nq, "c");
  circuit.x(0);

  circuit.initialLayout[0] = 1;
  circuit.initialLayout[1] = 0;
  circuit.setLogicalQubitGarbage(1);
  circuit.outputPermutation.erase(0);
  circuit.outputPermutation[1] = 0;

  for (std::size_t i = 0; i < nq; ++i) {
    circuit.h(1);
    if (hiddenString.test(i)) {
      circuit.cz(1, 0);
    }
    circuit.h(1);
    circuit.measure(1, i);
    if (i < nq - 1) {
      circuit.reset(1);
    }
  }
}

[[nodiscard]] auto createExactPhase(const Qubit nq, std::mt19937_64& generator)
    -> fp {
  const std::uint64_t max = 1ULL << nq;
  auto distribution = std::uniform_int_distribution<std::uint64_t>(0, max - 1);
  std::uint64_t theta = 0;
  while (theta == 0) {
    theta = distribution(generator);
  }
  fp lambda = 0.;
  for (std::size_t i = 0; i < nq; ++i) {
    if ((theta & (1ULL << (nq - i - 1))) != 0) {
      lambda += 1. / static_cast<double>(1ULL << i);
    }
  }
  return lambda;
}

[[nodiscard]] auto createInexactPhase(const Qubit nq,
                                      std::mt19937_64& generator) -> fp {
  const std::uint64_t max = 1ULL << (nq + 1);
  auto distribution = std::uniform_int_distribution<std::uint64_t>(0, max - 1);
  std::uint64_t theta = 0;
  while ((theta & 1U) == 0) {
    theta = distribution(generator);
  }
  fp lambda = 0.;
  for (std::size_t i = 0; i <= nq; ++i) {
    if ((theta & (1ULL << (nq - i))) != 0) {
      lambda += 1. / static_cast<double>(1ULL << i);
    }
  }
  return lambda;
}

auto constructIterativeQPECircuit(QuantumComputation& circuit, const fp lambda,
                                  const Qubit nq) -> void {
  auto name = std::ostringstream{};
  name << "iterative_qpe_" << nq << "_";
  name.precision(std::numeric_limits<fp>::digits10);
  name << lambda;
  circuit.setName(name.str());
  circuit.addQubitRegister(1, "psi");
  circuit.addQubitRegister(1, "q");
  circuit.addClassicalRegister(nq, "c");
  circuit.gphase(lambda);
  circuit.x(0);

  circuit.initialLayout[0] = 1;
  circuit.initialLayout[1] = 0;
  circuit.setLogicalQubitGarbage(1);
  circuit.outputPermutation.erase(0);
  circuit.outputPermutation[1] = 0;

  for (Qubit i = 0; i < nq; ++i) {
    circuit.h(1);
    const auto angle =
        std::remainder(static_cast<double>(1ULL << (nq - 1 - i)) * lambda, 2.0);
    circuit.cp(angle * qc::PI, 1, 0);
    for (std::size_t j = 0; j < i; ++j) {
      const auto phase = -qc::PI / static_cast<double>(1ULL << (i - j));
      circuit.if_(qc::P, 1, j, true, qc::Eq, {phase});
    }
    circuit.h(1);
    circuit.measure(1, i);
    if (i < nq - 1) {
      circuit.reset(1);
    }
  }
}
} // namespace

auto createGHZState(const qc::Qubit nq) -> qc::QuantumComputation {
  auto circuit = QuantumComputation(nq, nq);
  circuit.setName("ghz_" + std::to_string(nq));
  const auto top = nq - 1;
  circuit.h(top);
  for (Qubit i = 0; i < top; ++i) {
    circuit.cx(top, i);
  }
  return circuit;
}

auto createGrover(const qc::Qubit nq, const GroverBitString& targetValue)
    -> qc::QuantumComputation {
  auto circuit = QuantumComputation{};
  constructGroverCircuit(circuit, nq, targetValue);
  return circuit;
}

auto createGrover(const qc::Qubit nq, const std::size_t seed)
    -> qc::QuantumComputation {
  auto generator = createGenerator(seed);
  auto circuit = QuantumComputation{};
  constructGroverCircuit(circuit, nq, generateGroverTarget(nq, generator));
  return circuit;
}

auto createQFT(const qc::Qubit nq, const bool includeMeasurements)
    -> qc::QuantumComputation {
  auto circuit = QuantumComputation(nq, nq);
  circuit.setName("qft_" + std::to_string(nq));
  if (nq == 0) {
    return circuit;
  }
  for (Qubit i = 0; i < nq; ++i) {
    for (Qubit j = i; j > 0; --j) {
      const auto target = i - j;
      if (j == 1) {
        circuit.cs(i, target);
      } else if (j == 2) {
        circuit.ct(i, target);
      } else {
        circuit.cp(qc::PI / std::pow(2., j), i, target);
      }
    }
    circuit.h(i);
  }
  if (includeMeasurements) {
    for (Qubit i = 0; i < nq; ++i) {
      circuit.measure(i, nq - 1 - i);
      circuit.outputPermutation[i] = nq - 1 - i;
    }
  } else {
    for (Qubit i = 0; i < nq / 2; ++i) {
      circuit.swap(i, nq - 1 - i);
    }
    for (Qubit i = 0; i < nq; ++i) {
      circuit.outputPermutation[i] = nq - 1 - i;
    }
  }
  return circuit;
}

auto createIterativeBernsteinVazirani(
    const BernsteinVaziraniBitString& hiddenString, const qc::Qubit nq)
    -> qc::QuantumComputation {
  auto circuit = QuantumComputation{};
  constructIterativeBernsteinVaziraniCircuit(circuit, hiddenString, nq);
  return circuit;
}

auto createIterativeQFT(const qc::Qubit nq) -> qc::QuantumComputation {
  auto circuit = QuantumComputation(0, nq);
  circuit.setName("iterative_qft_" + std::to_string(nq));
  if (nq == 0) {
    return circuit;
  }
  circuit.addQubitRegister(1U);
  for (Qubit i = 0; i < nq; ++i) {
    for (Qubit j = 1; j <= i; ++j) {
      const auto bit = nq - j;
      if (j == i) {
        circuit.if_(qc::S, 0, bit);
      } else if (j == i - 1) {
        circuit.if_(qc::T, 0, bit);
      } else {
        circuit.if_(qc::P, 0, bit, true, qc::Eq,
                    {qc::PI / std::pow(2., i - j + 1)});
      }
    }
    circuit.h(0);
    circuit.measure(0, nq - 1 - i);
    if (i < nq - 1) {
      circuit.reset(0);
    }
  }
  return circuit;
}

auto createIterativeQPE(const qc::Qubit nq, const bool exact,
                        const std::size_t seed) -> qc::QuantumComputation {
  auto generator = createGenerator(seed);
  const auto lambda = exact ? createExactPhase(nq, generator)
                            : createInexactPhase(nq, generator);
  auto circuit = QuantumComputation{};
  constructIterativeQPECircuit(circuit, lambda, nq);
  return circuit;
}
} // namespace ddsim::detail

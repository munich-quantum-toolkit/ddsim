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

#include "ir/Definitions.hpp"

#include <bitset>
#include <cstddef>

namespace qc {
class QuantumComputation;
} // namespace qc

namespace ddsim::detail {
using BernsteinVaziraniBitString = std::bitset<4096>;
using GroverBitString = std::bitset<128>;

/// Creates a GHZ-state preparation circuit.
[[nodiscard]] auto createGHZState(qc::Qubit nq) -> qc::QuantumComputation;
/// Creates a Grover circuit for a specified target value.
[[nodiscard]] auto createGrover(qc::Qubit nq,
                                const GroverBitString& targetValue)
    -> qc::QuantumComputation;
/// Creates a Grover circuit for a target value generated from a seed.
[[nodiscard]] auto createGrover(qc::Qubit nq, std::size_t seed = 0)
    -> qc::QuantumComputation;
/// Creates a quantum Fourier transform circuit.
[[nodiscard]] auto createQFT(qc::Qubit nq, bool includeMeasurements = true)
    -> qc::QuantumComputation;
/// Creates an iterative Bernstein-Vazirani circuit.
[[nodiscard]] auto
createIterativeBernsteinVazirani(const BernsteinVaziraniBitString& hiddenString,
                                 qc::Qubit nq) -> qc::QuantumComputation;
/// Creates an iterative quantum Fourier transform circuit.
[[nodiscard]] auto createIterativeQFT(qc::Qubit nq) -> qc::QuantumComputation;
/// Creates an iterative quantum phase estimation circuit.
[[nodiscard]] auto createIterativeQPE(qc::Qubit nq, bool exact = true,
                                      std::size_t seed = 0)
    -> qc::QuantumComputation;
} // namespace ddsim::detail

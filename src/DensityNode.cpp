/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DensityNode.hpp"

#include "dd/Complex.hpp"
#include "dd/ComplexNumbers.hpp"
#include "dd/ComplexValue.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/MemoryManager.hpp"
#include "dd/RealNumber.hpp"
#include "ir/Definitions.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_set>
#include <utility>

namespace dd::ddsim {

///-----------------------------------------------------------------------------
///                         \n dNode methods \n
///-----------------------------------------------------------------------------

void dNode::setDensityMatrixNodeFlag(const bool densityMatrix) noexcept {
  if (densityMatrix) {
    flags = (flags | static_cast<std::uint8_t>(8U));
  } else {
    flags = (flags & static_cast<std::uint8_t>(~8U));
  }
}

std::uint8_t dNode::alignDensityNodeNode(dNode*& p) noexcept {
  const auto flags = static_cast<std::uint8_t>(getDensityMatrixTempFlags(p));
  // Get an aligned node
  alignDensityNode(p);

  if (dNode::isTerminal(p)) {
    return 0U;
  }

  if (isNonReduceTempFlagSet(flags) && !isConjugateTempFlagSet(flags)) {
    // nothing more to do for first edge path (inherited by all child paths)
    return flags;
  }

  if (!isConjugateTempFlagSet(flags)) {
    p->e[2].w = dd::ComplexNumbers::conj(p->e[2].w);
    setConjugateTempFlagTrue(p->e[2].p);
    // Mark the first edge
    setNonReduceTempFlagTrue(p->e[1].p);

    for (auto& edge : p->e) {
      setDensityMatTempFlagTrue(edge.p);
    }

  } else {
    std::swap(p->e[2], p->e[1]);
    for (auto& edge : p->e) {
      edge.w = dd::ComplexNumbers::conj(edge.w);
      setConjugateTempFlagTrue(edge.p);
      setDensityMatTempFlagTrue(edge.p);
    }
  }
  return flags;
}

void dNode::getAlignedNodeRevertModificationsOnSubEdges(dNode* p) noexcept {
  // Get an aligned node and revert the modifications on the sub edges
  alignDensityNode(p);

  for (auto& edge : p->e) {
    // remove the set properties from the node pointers of edge.p->e
    alignDensityNode(edge.p);
  }

  if (isNonReduceTempFlagSet(p->flags) && !isConjugateTempFlagSet(p->flags)) {
    // nothing more to do for a first edge path
    return;
  }

  if (!isConjugateTempFlagSet(p->flags)) {
    p->e[2].w = dd::ComplexNumbers::conj(p->e[2].w);
    return;
  }
  for (auto& edge : p->e) {
    edge.w = dd::ComplexNumbers::conj(edge.w);
  }
  std::swap(p->e[2], p->e[1]);
}

void dNode::applyDmChangesToNode(dNode*& p) noexcept {
  if (isDensityMatrixTempFlagSet(p)) {
    const auto tmp = alignDensityNodeNode(p);
    if (p == nullptr) {
      return;
    }
    assert(getDensityMatrixTempFlags(p->flags) == 0);
    p->flags = p->flags | tmp;
  }
}

void dNode::revertDmChangesToNode(dNode*& p) noexcept {
  if (!dNode::isTerminal(p) && isDensityMatrixTempFlagSet(p->flags)) {
    getAlignedNodeRevertModificationsOnSubEdges(p);
    p->unsetTempDensityMatrixFlags();
  }
}

///-----------------------------------------------------------------------------
///                    \n General purpose dEdge methods \n
///-----------------------------------------------------------------------------

auto dEdge::size() const -> std::size_t {
  static constexpr std::size_t NODECOUNT_BUCKETS = 200000U;
  static std::unordered_set<const dNode*> visited{NODECOUNT_BUCKETS};
  visited.max_load_factor(10);
  visited.clear();
  return size(visited);
}

auto dEdge::size(std::unordered_set<const dNode*>& visited) const
    -> std::size_t {
  visited.emplace(p);
  std::size_t sum = 1U;
  if (!isTerminal()) {
    for (const auto& e : p->e) {
      if (!visited.contains(e.p)) {
        sum += e.size(visited);
      }
    }
  }
  return sum;
}

void dEdge::mark() const noexcept {
  w.mark();
  if (isTerminal() || p->isMarked()) {
    return;
  }
  p->mark();
  for (const dEdge& e : p->e) {
    e.mark();
  }
}

void dEdge::unmark() const noexcept {
  w.unmark();
  if (isTerminal() || !p->isMarked()) {
    return;
  }
  p->unmark();
  for (const dEdge& e : p->e) {
    e.unmark();
  }
}

///-----------------------------------------------------------------------------
///                    \n Normalization (matrix variant) \n
///-----------------------------------------------------------------------------

auto dEdge::normalize(dNode* p, const std::array<dEdge, dd::NEDGE>& e,
                      dd::MemoryManager& mm, dd::ComplexNumbers& cn) -> dEdge {
  assert(p != nullptr && "Node pointer passed to normalize is null.");
  const auto zero = std::array{e[0].w.exactlyZero(), e[1].w.exactlyZero(),
                               e[2].w.exactlyZero(), e[3].w.exactlyZero()};

  if (std::ranges::all_of(zero, [](auto b) { return b; })) {
    mm.returnEntry(*p);
    return dEdge::zero();
  }

  const auto weights = std::array{static_cast<dd::ComplexValue>(e[0].w),
                                  static_cast<dd::ComplexValue>(e[1].w),
                                  static_cast<dd::ComplexValue>(e[2].w),
                                  static_cast<dd::ComplexValue>(e[3].w)};

  std::optional<std::size_t> argMax = std::nullopt;
  dd::fp maxMag2 = 0.;
  auto maxVal = dd::Complex::one();
  // determine max amplitude
  for (auto i = 0U; i < dd::NEDGE; ++i) {
    if (zero[i]) {
      p->e[i] = dEdge::zero();
      continue;
    }
    const auto& w = weights[i];
    if (!argMax.has_value()) {
      argMax = i;
      maxMag2 = w.mag2();
      maxVal = e[i].w;
    } else {
      if (const auto mag2 = w.mag2(); mag2 - maxMag2 > dd::RealNumber::eps) {
        argMax = i;
        maxMag2 = mag2;
        maxVal = e[i].w;
      }
    }
  }
  assert(argMax.has_value() && "argMax should have been set by now");

  const auto argMaxValue = *argMax;
  const auto argMaxWeight = weights[argMaxValue];
  for (auto i = 0U; i < dd::NEDGE; ++i) {
    if (zero[i]) {
      continue;
    }
    if (i == argMaxValue) {
      p->e[i] = {.p = e[i].p, .w = dd::Complex::one()};
      continue;
    }
    p->e[i] = {.p = e[i].p, .w = cn.lookup(weights[i] / argMaxWeight)};
    if (p->e[i].w.exactlyZero()) {
      p->e[i].p = dNode::getTerminal();
    }
  }
  return dEdge{.p = p, .w = maxVal};
}

///-----------------------------------------------------------------------------
///                 \n Methods for density matrix DDs \n
///-----------------------------------------------------------------------------

auto dEdge::getSparseProbabilityVector(const std::size_t numQubits,
                                       const dd::fp threshold) const
    -> dd::SparsePVec {
  if (numQubits == 0U) {
    return {{0, static_cast<std::complex<dd::fp>>(w).real()}};
  }

  auto e = *this;
  dEdge::alignDensityEdge(e);

  auto probabilities = dd::SparsePVec{};
  e.traverseDiagonal(
      1, 0,
      [&probabilities](const std::size_t i, const dd::fp& prob) {
        probabilities[i] = prob;
      },
      numQubits, threshold);
  return probabilities;
}

auto dEdge::getSparseProbabilityVectorStrKeys(const std::size_t numQubits,
                                              const dd::fp threshold) const
    -> dd::SparsePVecStrKeys {
  if (numQubits == 0U) {
    return {{"0", static_cast<std::complex<dd::fp>>(w).real()}};
  }

  auto e = *this;
  dEdge::alignDensityEdge(e);
  const auto nqubits = static_cast<std::size_t>(e.p->v) + 1U;

  auto probabilities = dd::SparsePVecStrKeys{};
  e.traverseDiagonal(
      1, 0,
      [&probabilities, &nqubits](const std::size_t i, const dd::fp& prob) {
        probabilities[dd::intToBinaryString(i, nqubits)] = prob;
      },
      numQubits, threshold);
  return probabilities;
}

void dEdge::traverseDiagonal(const dd::fp& prob, const std::size_t i,
                             const dd::ProbabilityFunc& f,
                             const std::size_t level,
                             const dd::fp threshold) const {
  // calculate new accumulated probability
  const auto c = static_cast<std::complex<dd::fp>>(w);
  const auto val = prob * c.real();

  if (val < threshold) {
    return;
  }

  if (level == 0) {
    assert(isTerminal());
    f(i, val);
    return;
  }

  const auto nextLevel = static_cast<dd::Qubit>(level - 1U);
  if (isTerminal() || p->v < nextLevel) {
    traverseDiagonal(prob, i, f, nextLevel, threshold);
    traverseDiagonal(prob, i | (1ULL << nextLevel), f, nextLevel, threshold);
    return;
  }

  if (auto& e = p->e[0]; !e.w.exactlyZero()) {
    e.traverseDiagonal(val, i, f, nextLevel, threshold);
  }
  if (auto& e = p->e[3]; !e.w.exactlyZero()) {
    e.traverseDiagonal(val, i | (1ULL << nextLevel), f, nextLevel, threshold);
  }
}

///-----------------------------------------------------------------------------
///                    \n dCachedEdge normalization \n
///-----------------------------------------------------------------------------

auto dCachedEdge::normalize(dNode* p,
                            const std::array<dCachedEdge, dd::NEDGE>& e,
                            dd::MemoryManager& mm, dd::ComplexNumbers& cn)
    -> dCachedEdge {
  assert(p != nullptr && "Node pointer passed to normalize is null.");
  const auto zero =
      std::array{e[0].w.approximatelyZero(), e[1].w.approximatelyZero(),
                 e[2].w.approximatelyZero(), e[3].w.approximatelyZero()};

  if (std::ranges::all_of(zero, [](auto b) { return b; })) {
    mm.returnEntry(*p);
    return dCachedEdge::zero();
  }

  std::optional<std::size_t> argMax = std::nullopt;
  dd::fp maxMag2 = 0.;
  dd::ComplexValue maxVal = 1.;
  // determine max amplitude
  for (auto i = 0U; i < dd::NEDGE; ++i) {
    if (zero[i]) {
      continue;
    }
    const auto& w = e[i].w;
    if (!argMax.has_value()) {
      argMax = i;
      maxMag2 = w.mag2();
      maxVal = w;
    } else {
      if (const auto mag2 = w.mag2(); mag2 - maxMag2 > dd::RealNumber::eps) {
        argMax = i;
        maxMag2 = mag2;
        maxVal = w;
      }
    }
  }
  assert(argMax.has_value() && "argMax should have been set by now");

  const auto argMaxValue = *argMax;
  for (auto i = 0U; i < dd::NEDGE; ++i) {
    // The approximation below is really important for numerical stability.
    // An exactly zero check will lead to numerical instabilities.
    if (zero[i]) {
      p->e[i] = dEdge::zero();
      continue;
    }
    if (i == argMaxValue) {
      p->e[i] = {.p = e[i].p, .w = dd::Complex::one()};
      continue;
    }
    p->e[i] = {.p = e[i].p, .w = cn.lookup(e[i].w / maxVal)};
    if (p->e[i].w.exactlyZero()) {
      p->e[i].p = dNode::getTerminal();
    }
  }
  return dCachedEdge{p, maxVal};
}

} // namespace dd::ddsim

///-----------------------------------------------------------------------------
///                         \n Hash related code \n
///-----------------------------------------------------------------------------

namespace std {

std::size_t
hash<dd::ddsim::dEdge>::operator()(const dd::ddsim::dEdge& e) const noexcept {
  const auto h1 = dd::murmur64(reinterpret_cast<std::size_t>(e.p));
  const auto h2 = std::hash<dd::Complex>{}(e.w);
  auto h3 = qc::combineHash(h1, h2);
  if (e.isTerminal()) {
    return h3;
  }
  assert(dd::ddsim::dNode::isDensityMatrixTempFlagSet(e.p) == false);
  const auto h4 = dd::ddsim::dNode::getDensityMatrixTempFlags(e.p->flags);
  h3 = qc::combineHash(h3, h4);
  return h3;
}

std::size_t hash<dd::ddsim::dCachedEdge>::operator()(
    const dd::ddsim::dCachedEdge& e) const noexcept {
  const auto h1 = dd::murmur64(reinterpret_cast<std::size_t>(e.p));
  const auto h2 = std::hash<dd::ComplexValue>{}(e.w);
  return qc::combineHash(h1, h2);
}

} // namespace std

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
 * @file DensityNode.hpp
 * @brief Density-matrix DD node, edge and cached-edge types.
 *
 * @details This is a self-contained copy of the density-matrix support that
 * used to live in the MQT Core DD package (`dd::dNode`, `dd::dEdge`,
 * `dd::dCachedEdge`) before it was removed in
 * https://github.com/munich-quantum-toolkit/core/pull/1466. It lives in the
 * separate namespace `dd::ddsim` so that it can coexist with a Core version
 * that still provides the old types.
 */

#pragma once

#include "dd/Complex.hpp"
#include "dd/ComplexValue.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/Edge.hpp"
#include "dd/Node.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace dd::ddsim {

struct dNode; // NOLINT(readability-identifier-naming)

/**
 * @brief A weighted edge pointing to a density-matrix DD node.
 */
struct dEdge { // NOLINT(readability-identifier-naming)
  dNode* p;
  dd::Complex w;

  constexpr bool operator==(const dEdge& other) const {
    return p == other.p && w.approximatelyEquals(other.w);
  }
  constexpr bool operator!=(const dEdge& other) const {
    return !operator==(other);
  }

  static constexpr dEdge zero() { return terminal(dd::Complex::zero()); }
  static constexpr dEdge one() { return terminal(dd::Complex::one()); }
  [[nodiscard]] static constexpr dEdge terminal(const dd::Complex& w);

  [[nodiscard]] static constexpr bool trackingRequired(const dEdge& e) {
    return !e.isTerminal() || !dd::constants::isStaticNumber(e.w.r) ||
           !dd::constants::isStaticNumber(e.w.i);
  }

  [[nodiscard]] bool isTerminal() const;
  [[nodiscard]] bool isZeroTerminal() const {
    return isTerminal() && w.exactlyZero();
  }
  [[nodiscard]] bool isOneTerminal() const {
    return isTerminal() && w.exactlyOne();
  }

  [[nodiscard]] bool isIdentity(bool upToGlobalPhase = true) const {
    if (!isTerminal()) {
      return false;
    }
    if (upToGlobalPhase) {
      return !w.exactlyZero();
    }
    return w.exactlyOne();
  }

  /**
   * @brief Get the size of the DD (number of nodes including the terminal).
   */
  [[nodiscard]] std::size_t size() const;

  /// @brief Mark the edge (and its sub-DD) as used.
  void mark() const noexcept;
  /// @brief Unmark the edge (and its sub-DD).
  void unmark() const noexcept;

  /**
   * @brief Get a normalized density-matrix DD from a fresh node and its edges.
   */
  static auto normalize(dNode* p, const std::array<dEdge, dd::NEDGE>& e,
                        dd::MemoryManager& mm, dd::ComplexNumbers& cn) -> dEdge;

  [[maybe_unused]] static void setDensityConjugateTrue(dEdge& e);
  [[maybe_unused]] static void setFirstEdgeDensityPathTrue(dEdge& e);
  static void setDensityMatrixTrue(dEdge& e);
  static void alignDensityEdge(dEdge& e);
  static void revertDmChangesToEdges(dEdge& x, dEdge& y);
  static void revertDmChangesToEdge(dEdge& x);
  static void applyDmChangesToEdges(dEdge& x, dEdge& y);
  static void applyDmChangesToEdge(dEdge& x);

  /**
   * @brief Get the sparse probability vector for the underlying density matrix.
   */
  [[nodiscard]] dd::SparsePVec
  getSparseProbabilityVector(std::size_t numQubits,
                             dd::fp threshold = 0.) const;

  /**
   * @brief Get the sparse probability vector using strings as keys.
   */
  [[nodiscard]] dd::SparsePVecStrKeys
  getSparseProbabilityVectorStrKeys(std::size_t numQubits,
                                    dd::fp threshold = 0.) const;

private:
  [[nodiscard]] std::size_t
  size(std::unordered_set<const dNode*>& visited) const;

  void traverseDiagonal(const dd::fp& prob, std::size_t i,
                        dd::ProbabilityFunc f, std::size_t level,
                        dd::fp threshold = 0.) const;
};

using DensityMatrixDD = dEdge;

/**
 * @brief A density-matrix DD node with a cached (non-canonical) edge weight.
 */
struct dCachedEdge { // NOLINT(readability-identifier-naming)
  dNode* p{};
  dd::ComplexValue w;

  dCachedEdge() = default;
  dCachedEdge(dNode* n, const dd::ComplexValue& v) : p(n), w(v) {}
  dCachedEdge(dNode* n, const dd::Complex& c)
      : p(n), w(static_cast<dd::ComplexValue>(c)) {}

  bool operator==(const dCachedEdge& other) const {
    return p == other.p && w.approximatelyEquals(other.w);
  }
  bool operator!=(const dCachedEdge& other) const { return !operator==(other); }

  [[nodiscard]] static dCachedEdge terminal(const dd::ComplexValue& w);
  [[nodiscard]] static dCachedEdge terminal(const std::complex<dd::fp>& w);
  [[nodiscard]] static dCachedEdge terminal(const dd::Complex& w);
  [[nodiscard]] static dCachedEdge zero() {
    return terminal(dd::ComplexValue(0.));
  }
  [[nodiscard]] static dCachedEdge one() {
    return terminal(dd::ComplexValue(1.));
  }

  [[nodiscard]] bool isTerminal() const;

  [[nodiscard]] bool isIdentity(bool upToGlobalPhase = true) const {
    if (!isTerminal()) {
      return false;
    }
    if (upToGlobalPhase) {
      return !w.exactlyZero();
    }
    return w.exactlyOne();
  }

  /**
   * @brief Get a normalized density-matrix DD from a fresh node and its edges.
   */
  static auto normalize(dNode* p, const std::array<dCachedEdge, dd::NEDGE>& e,
                        dd::MemoryManager& mm, dd::ComplexNumbers& cn)
      -> dCachedEdge;
};

/**
 * @brief A density-matrix DD node.
 * @details Data Layout (8)|(2|2|4)|(24|24|24|24) = 112B
 */
struct dNode final : dd::NodeBase { // NOLINT(readability-identifier-naming)
  std::array<dEdge, dd::NEDGE> e{}; // edges out of this node

  /// Getter for the next object
  [[nodiscard]] dNode* next() const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<dNode*>(next_);
  }
  /// Getter for the terminal object
  static constexpr dNode* getTerminal() noexcept { return nullptr; }

  [[nodiscard]] [[maybe_unused]] static constexpr bool
  tempDensityMatrixFlagsEqual(const std::uint8_t a,
                              const std::uint8_t b) noexcept {
    return getDensityMatrixTempFlags(a) == getDensityMatrixTempFlags(b);
  }

  [[nodiscard]] static constexpr bool
  isConjugateTempFlagSet(const std::uintptr_t p) noexcept {
    return (p & (1ULL << 0)) != 0U;
  }
  [[nodiscard]] static constexpr bool
  isNonReduceTempFlagSet(const std::uintptr_t p) noexcept {
    return (p & (1ULL << 1)) != 0U;
  }
  [[nodiscard]] static constexpr bool
  isDensityMatrixTempFlagSet(const std::uintptr_t p) noexcept {
    return (p & (1ULL << 2)) != 0U;
  }
  [[nodiscard]] static bool
  isDensityMatrixNode(const std::uintptr_t p) noexcept {
    return (p & (1ULL << 3)) != 0U;
  }

  [[nodiscard]] static bool isConjugateTempFlagSet(const dNode* p) noexcept {
    return isConjugateTempFlagSet(reinterpret_cast<std::uintptr_t>(p));
  }
  [[nodiscard]] static bool isNonReduceTempFlagSet(const dNode* p) noexcept {
    return isNonReduceTempFlagSet(reinterpret_cast<std::uintptr_t>(p));
  }
  [[nodiscard]] static bool
  isDensityMatrixTempFlagSet(const dNode* p) noexcept {
    return isDensityMatrixTempFlagSet(reinterpret_cast<std::uintptr_t>(p));
  }
  [[nodiscard]] static bool isDensityMatrixNode(const dNode* p) noexcept {
    return isDensityMatrixNode(reinterpret_cast<std::uintptr_t>(p));
  }

  static void setConjugateTempFlagTrue(dNode*& p) noexcept {
    p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) |
                                 (1ULL << 0));
  }
  static void setNonReduceTempFlagTrue(dNode*& p) noexcept {
    p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) |
                                 (1ULL << 1));
  }
  static void setDensityMatTempFlagTrue(dNode*& p) noexcept {
    p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) |
                                 (1ULL << 2));
  }
  static void alignDensityNode(dNode*& p) noexcept {
    p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) & (~7ULL));
  }

  [[nodiscard]] static std::uintptr_t
  getDensityMatrixTempFlags(dNode*& p) noexcept {
    return getDensityMatrixTempFlags(reinterpret_cast<std::uintptr_t>(p));
  }
  [[nodiscard]] static constexpr std::uintptr_t
  getDensityMatrixTempFlags(const std::uintptr_t a) noexcept {
    return a & (7ULL);
  }

  constexpr void unsetTempDensityMatrixFlags() noexcept {
    flags = flags & static_cast<std::uint8_t>(~7U);
  }

  void setDensityMatrixNodeFlag(bool densityMatrix) noexcept;

  static std::uint8_t alignDensityNodeNode(dNode*& p) noexcept;

  static void getAlignedNodeRevertModificationsOnSubEdges(dNode* p) noexcept;

  static void applyDmChangesToNode(dNode*& p) noexcept;

  static void revertDmChangesToNode(dNode*& p) noexcept;
};

/// Reinterpret a Core matrix edge as a density-matrix edge.
inline dEdge densityFromMatrixEdge(const dd::mEdge& e) {
  return dEdge{reinterpret_cast<dNode*>(e.p), e.w};
}

///-----------------------------------------------------------------------------
/// Inline definitions that require the complete `dNode` type
///-----------------------------------------------------------------------------

constexpr dEdge dEdge::terminal(const dd::Complex& w) {
  return dEdge{dNode::getTerminal(), w};
}

inline bool dEdge::isTerminal() const { return dNode::isTerminal(p); }

inline void dEdge::setDensityConjugateTrue(dEdge& e) {
  dNode::setConjugateTempFlagTrue(e.p);
}
inline void dEdge::setFirstEdgeDensityPathTrue(dEdge& e) {
  dNode::setNonReduceTempFlagTrue(e.p);
}
inline void dEdge::setDensityMatrixTrue(dEdge& e) {
  dNode::setDensityMatTempFlagTrue(e.p);
}
inline void dEdge::alignDensityEdge(dEdge& e) { dNode::alignDensityNode(e.p); }
inline void dEdge::revertDmChangesToEdges(dEdge& x, dEdge& y) {
  revertDmChangesToEdge(x);
  revertDmChangesToEdge(y);
}
inline void dEdge::revertDmChangesToEdge(dEdge& x) {
  dNode::revertDmChangesToNode(x.p);
}
inline void dEdge::applyDmChangesToEdges(dEdge& x, dEdge& y) {
  applyDmChangesToEdge(x);
  applyDmChangesToEdge(y);
}
inline void dEdge::applyDmChangesToEdge(dEdge& x) {
  dNode::applyDmChangesToNode(x.p);
}

inline dCachedEdge dCachedEdge::terminal(const dd::ComplexValue& w) {
  return dCachedEdge{dNode::getTerminal(), w};
}
inline dCachedEdge dCachedEdge::terminal(const std::complex<dd::fp>& w) {
  return dCachedEdge{dNode::getTerminal(), static_cast<dd::ComplexValue>(w)};
}
inline dCachedEdge dCachedEdge::terminal(const dd::Complex& w) {
  return terminal(static_cast<dd::ComplexValue>(w));
}
inline bool dCachedEdge::isTerminal() const { return dNode::isTerminal(p); }

} // namespace dd::ddsim

template <> struct std::hash<dd::ddsim::dEdge> {
  std::size_t operator()(const dd::ddsim::dEdge& e) const noexcept;
};

template <> struct std::hash<dd::ddsim::dCachedEdge> {
  std::size_t operator()(const dd::ddsim::dCachedEdge& e) const noexcept;
};

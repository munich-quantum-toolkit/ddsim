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
 * @file DensityDDPackage.hpp
 * @brief Self-contained engine for density-matrix decision diagrams.
 *
 * @details This reconstructs the density-matrix operations that used to be
 * member functions of the MQT Core `dd::Package` before they were removed in
 * https://github.com/munich-quantum-toolkit/core/pull/1466. The engine owns the
 * density-matrix node space (memory manager, unique table, compute tables) and
 * borrows a `dd::Package` for the (surviving) matrix/complex-number
 * infrastructure it needs (gate construction, conjugate transpose, complex
 * number pool).
 *
 * The density DD reuses matrix nodes and complex numbers from the borrowed
 * package (via the layout-compatible `densityFromMatrixEdge` reinterpretation).
 * To keep those shared entries alive across the package's own garbage
 * collection, every density root is additionally registered in the package's
 * matrix root set (again via reinterpretation). The density node space itself
 * is collected by this engine's own @ref garbageCollect.
 */

#pragma once

#include "DensityComputeTable.hpp"
#include "DensityNode.hpp"
#include "DensityUniqueTable.hpp"
#include "dd/ComplexValue.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/DDpackageConfig.hpp"
#include "dd/MemoryManager.hpp"
#include "dd/Node.hpp"
#include "dd/Package.hpp"
#include "dd/UnaryComputeTable.hpp"

#include <array>
#include <cstddef>
#include <random>
#include <unordered_map>
#include <vector>

namespace dd::ddsim {

/// Bucket sizes for the density-matrix compute and unique tables.
struct DensityDDPackageConfig {
  std::size_t utDmNumBucket = 65536U;
  std::size_t utDmInitialAllocationSize = 4096U;
  std::size_t ctDmDmMultNumBucket = 16384U;
  std::size_t ctDmAddNumBucket = 16384U;
  std::size_t ctDmTraceNumBucket = 4096U;
};

/// Configuration for the (borrowed) matrix/vector `dd::Package` used by the
/// deterministic (density-matrix) noise-aware simulator.
constexpr auto DENSITY_MATRIX_SIMULATOR_DD_PACKAGE_CONFIG = []() {
  dd::DDPackageConfig config{};
  config.utMatNumBucket = 16384U;
  config.ctMatAddNumBucket = 4096U;
  config.ctVecAddNumBucket = 4096U;
  config.ctMatConjTransNumBucket = 4096U;
  config.ctMatMatMultNumBucket = 1U;
  config.ctMatVecMultNumBucket = 1U;
  config.utVecNumBucket = 1U;
  config.utVecInitialAllocationSize = 1U;
  config.utMatInitialAllocationSize = 1U;
  config.ctVecKronNumBucket = 1U;
  config.ctMatKronNumBucket = 1U;
  config.ctMatTraceNumBucket = 1U;
  config.ctVecInnerProdNumBucket = 1U;
  config.ctVecAddMagNumBucket = 1U;
  config.ctMatAddMagNumBucket = 1U;
  config.ctVecConjNumBucket = 1U;
  return config;
}();

/// Configuration for the (borrowed) matrix/vector `dd::Package` used by the
/// stochastic noise-aware simulator.
constexpr auto STOCHASTIC_NOISE_SIMULATOR_DD_PACKAGE_CONFIG = []() {
  dd::DDPackageConfig config{};
  config.ctVecAddMagNumBucket = 1U;
  config.ctMatAddMagNumBucket = 1U;
  config.ctVecConjNumBucket = 1U;
  return config;
}();

/// Reinterpret a density-matrix edge as a matrix edge (inverse of
/// @ref densityFromMatrixEdge). Only safe for aligned (untagged) edges.
inline dd::mEdge matrixFromDensityEdge(const dEdge& e) {
  return dd::mEdge{reinterpret_cast<dd::mNode*>(e.p), e.w};
}

/// Engine that provides density-matrix DD operations on top of a `dd::Package`.
class DensityDDPackage {
public:
  DensityDDPackage(dd::Package& package, std::size_t nqubits,
                   const DensityDDPackageConfig& config = {})
      : pkg(&package), dMemoryManager(dd::MemoryManager::create<dNode>(
                           config.utDmInitialAllocationSize)),
        dUniqueTable(dMemoryManager,
                     {.nVars = nqubits, .nBuckets = config.utDmNumBucket}),
        densityAdd(config.ctDmAddNumBucket),
        densityDensityMultiplication(config.ctDmDmMultNumBucket),
        densityTrace(config.ctDmTraceNumBucket) {}

  /**
   * @brief Construct the all-zero density operator \f$|0...0><0...0|\f$.
   */
  dEdge makeZeroDensityOperator(std::size_t n);

  /**
   * @brief Apply a matrix operation to a density matrix.
   */
  dEdge applyOperationToDensity(dEdge& e, const dd::mEdge& operation);

  /**
   * @brief Perform a collapsing measurement of a single qubit.
   */
  char measureOneCollapsing(dEdge& e, dd::Qubit index, std::mt19937_64& mt);

  /**
   * @brief Multiply two density-matrix DDs.
   */
  dEdge multiply(const dEdge& x, const dEdge& y,
                 bool generateDensityMatrix = false);

  /**
   * @brief Add two (cached) density-matrix DDs.
   */
  dCachedEdge add2(const dCachedEdge& x, const dCachedEdge& y, dd::Qubit var);

  /**
   * @brief Compute the trace of a density-matrix DD.
   */
  dd::ComplexValue trace(const dEdge& a, std::size_t numQubits);

  /**
   * @brief Create a normalized density-matrix node from a list of edges.
   */
  dEdge makeDDNode(dd::Qubit var, const std::array<dEdge, dd::NEDGE>& edges,
                   bool generateDensityMatrix = false);
  dCachedEdge makeDDNode(dd::Qubit var,
                         const std::array<dCachedEdge, dd::NEDGE>& edges,
                         bool generateDensityMatrix = false);

  /// Increase the reference count of a density DD.
  void incRef(const dEdge& e);
  /// Decrease the reference count of a density DD.
  void decRef(const dEdge& e);

  /// Trigger garbage collection of the density node space.
  bool garbageCollect(bool force = false);

  /// Number of active density-matrix nodes.
  [[nodiscard]] std::size_t computeActiveNodeCount() const;

  [[nodiscard]] dd::Package& package() const { return *pkg; }

private:
  dCachedEdge multiply2(const dEdge& x, const dEdge& y, dd::Qubit var,
                        bool generateDensityMatrix);

  dCachedEdge trace(const dEdge& a, const std::vector<bool>& eliminate,
                    std::size_t level, std::size_t alreadyEliminated = 0);

  dd::Package* pkg;
  dd::MemoryManager dMemoryManager;
  DensityUniqueTable dUniqueTable;
  DensityComputeTable<dCachedEdge, dCachedEdge, dCachedEdge> densityAdd;
  DensityComputeTable<dNode*, dNode*, dCachedEdge> densityDensityMultiplication;
  dd::UnaryComputeTable<dNode*, dCachedEdge> densityTrace;
  std::unordered_map<dEdge, std::size_t> dRoots;
};

} // namespace dd::ddsim

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
 * @file DensityComputeTable.hpp
 * @brief Compute table for caching results of density-matrix operations.
 *
 * @details Self-contained copy of the MQT Core `dd::ComputeTable` that keeps
 * the density-matrix specific hashing (accounting for the temporary flags
 * encoded in the node pointer) and the `useDensityMatrix` discrimination on
 * lookup. Both are required for the correctness of the reduced density-matrix
 * representation.
 */

#pragma once

#include "DensityNode.hpp"
#include "dd/statistics/TableStatistics.hpp"
#include "ir/Definitions.hpp"

#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace dd::ddsim {

/**
 * @brief Data structure for caching computed results of binary operations on
 * density-matrix DDs.
 */
template <class LeftOperandType, class RightOperandType, class ResultType>
class DensityComputeTable {
public:
  static constexpr std::size_t DEFAULT_NUM_BUCKETS = 16384U;

  explicit DensityComputeTable(
      const std::size_t numBuckets = DEFAULT_NUM_BUCKETS) {
    if ((numBuckets & (numBuckets - 1)) != 0) {
      throw std::invalid_argument("Number of buckets must be a power of two.");
    }
    stats.entrySize = sizeof(Entry);
    stats.numBuckets = numBuckets;
    valid = std::vector(numBuckets, false);
    table = std::vector<Entry>(numBuckets);
  }

  struct Entry {
    LeftOperandType leftOperand;
    RightOperandType rightOperand;
    ResultType result;
  };

  [[nodiscard]] std::size_t hash(const LeftOperandType& leftOperand,
                                 const RightOperandType& rightOperand) const {
    auto h1 = std::hash<LeftOperandType>{}(leftOperand);
    if constexpr (std::is_same_v<LeftOperandType, dNode*>) {
      if (!dNode::isTerminal(leftOperand)) {
        h1 = qc::combineHash(
            h1, dNode::getDensityMatrixTempFlags(leftOperand->flags));
      }
    }
    auto h2 = std::hash<RightOperandType>{}(rightOperand);
    if constexpr (std::is_same_v<RightOperandType, dNode*>) {
      if (!dNode::isTerminal(rightOperand)) {
        h2 = qc::combineHash(
            h2, dNode::getDensityMatrixTempFlags(rightOperand->flags));
      }
    }
    const auto hash = qc::combineHash(h1, h2);
    const auto mask = stats.numBuckets - 1;
    return hash & mask;
  }

  [[nodiscard]] const auto& getTable() const { return table; }
  [[nodiscard]] const auto& getStats() const noexcept { return stats; }

  void insert(const LeftOperandType& leftOperand,
              const RightOperandType& rightOperand, const ResultType& result) {
    const auto key = hash(leftOperand, rightOperand);
    if (valid[key]) {
      ++stats.collisions;
    } else {
      stats.trackInsert();
      valid[key] = true;
    }
    table[key] = {leftOperand, rightOperand, result};
  }

  ResultType* lookup(const LeftOperandType& leftOperand,
                     const RightOperandType& rightOperand,
                     [[maybe_unused]] const bool useDensityMatrix = false) {
    ResultType* result = nullptr;
    ++stats.lookups;
    const auto key = hash(leftOperand, rightOperand);
    if (!valid[key]) {
      return result;
    }

    auto& entry = table[key];
    if (entry.leftOperand != leftOperand) {
      return result;
    }
    if (entry.rightOperand != rightOperand) {
      return result;
    }

    if constexpr (std::is_same_v<RightOperandType, dNode*> ||
                  std::is_same_v<RightOperandType, dCachedEdge>) {
      // Since density matrices are reduced representations of matrices, a
      // density matrix may not be returned when a matrix is required and vice
      // versa
      if (!dNode::isTerminal(entry.result.p) &&
          dNode::isDensityMatrixNode(entry.result.p->flags) !=
              useDensityMatrix) {
        return result;
      }
    }
    ++stats.hits;
    return &entry.result;
  }

  void clear() { valid = std::vector(stats.numBuckets, false); }

  std::ostream& printStatistics(std::ostream& os = std::cout) const {
    return os << stats;
  }

private:
  std::vector<Entry> table;
  std::vector<bool> valid;
  dd::TableStatistics stats{};
};

} // namespace dd::ddsim

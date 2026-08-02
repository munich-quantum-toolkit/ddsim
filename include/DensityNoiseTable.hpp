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
 * @file DensityNoiseTable.hpp
 * @brief Data structure for caching computed results of noise operations
 *
 * @details Self-contained copy of the MQT Core `dd::DensityNoiseTable` that was
 * removed alongside the density-matrix support.
 */

#pragma once

#include "dd/DDDefinitions.hpp"
#include "dd/statistics/TableStatistics.hpp"

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

namespace dd::ddsim {

/**
 * @brief Data structure for caching computed results of noise operations
 * @tparam OperandType type of the operation's operand
 * @tparam ResultType type of the operation's result
 */
template <class OperandType, class ResultType>
class DensityNoiseTable { // todo: Inherit from UnaryComputerTable
public:
  /**
   * Default constructor
   * @param numBuckets Number of hash table buckets. Must be a power of two.
   */
  explicit DensityNoiseTable(const std::size_t numBuckets = 32768U) {
    // numBuckets must be a power of two
    if ((numBuckets & (numBuckets - 1)) != 0) {
      throw std::invalid_argument("Number of buckets must be a power of two.");
    }
    stats.entrySize = sizeof(Entry);
    stats.numBuckets = numBuckets;
    valid = std::vector(numBuckets, false);
    table = std::vector<Entry>(numBuckets);
  }

  struct Entry {
    OperandType operand;
    ResultType result;
    std::vector<dd::Qubit> usedQubits;
  };

  /// Get a reference to the table
  [[nodiscard]] const auto& getTable() const { return table; }

  /// Get a reference to the statistics
  [[nodiscard]] const auto& getStats() const noexcept { return stats; }

  [[nodiscard]] std::size_t
  hash(const OperandType& a, const std::vector<dd::Qubit>& usedQubits) const {
    std::size_t i = 0;
    for (const auto qubit : usedQubits) {
      i = (i << 3U) + (i * static_cast<std::size_t>(qubit)) +
          static_cast<std::size_t>(qubit);
    }
    const std::size_t mask = stats.numBuckets - 1;
    return (std::hash<OperandType>{}(a) + i) & mask;
  }

  void insert(const OperandType& operand, const ResultType& result,
              const std::vector<dd::Qubit>& usedQubits) {
    const auto key = hash(operand, usedQubits);
    if (valid[key]) {
      ++stats.collisions;
    } else {
      stats.trackInsert();
      valid[key] = true;
    }
    table[key] = {operand, result, usedQubits};
  }

  ResultType lookup(const OperandType& operand,
                    const std::vector<dd::Qubit>& usedQubits) {
    ResultType result{};
    ++stats.lookups;
    const auto key = hash(operand, usedQubits);

    if (!valid[key]) {
      return result;
    }

    auto& entry = table[key];
    if (entry.operand != operand) {
      return result;
    }
    if (entry.usedQubits != usedQubits) {
      return result;
    }
    ++stats.hits;
    return entry.result;
  }

  void clear() { valid = std::vector(stats.numBuckets, false); }

private:
  std::vector<Entry> table;
  std::vector<bool> valid;
  dd::TableStatistics stats{};
};

} // namespace dd::ddsim

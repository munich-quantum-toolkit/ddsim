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
 * @file StochasticNoiseOperationTable.hpp
 * @brief Data structure for caching computed results of stochastic operations
 *
 * @details Self-contained copy of the MQT Core
 * `dd::StochasticNoiseOperationTable` that was removed alongside the
 * density-matrix support. The number of cached operations is provided
 * explicitly instead of being derived from the (removed) noise `OpType`s.
 */

#pragma once

#include "dd/statistics/TableStatistics.hpp"
#include "ir/Definitions.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dd::ddsim {

template <class Edge> class StochasticNoiseOperationTable {
public:
  StochasticNoiseOperationTable(const std::size_t nv,
                                const std::size_t numberOfStochasticOperations)
      : nvars(nv), numberOfOperations(numberOfStochasticOperations),
        table(nv, std::vector<Edge>(numberOfStochasticOperations)) {
    stats.entrySize = sizeof(Edge);
    stats.numBuckets = nv * numberOfStochasticOperations;
  }

  /// Get a reference to the table
  [[nodiscard]] const auto& getTable() const { return table; }

  /// Get a reference to the statistics
  [[nodiscard]] const auto& getStats() const noexcept { return stats; }

  void resize(const std::size_t nq) {
    nvars = nq;
    table.resize(nvars, std::vector<Edge>(numberOfOperations));
  }

  void insert(std::uint8_t kind, qc::Qubit target, const Edge& r) {
    // Increase numberOfOperations if this assertion is hit for a valid kind.
    assert(kind < numberOfOperations);
    table.at(target).at(kind) = r;
    stats.trackInsert();
  }

  Edge* lookup(std::uint8_t kind, qc::Qubit target) {
    // Increase numberOfOperations if this assertion is hit for a valid kind.
    assert(kind < numberOfOperations);
    ++stats.lookups;
    auto& entry = table.at(target).at(kind);
    if (entry.w.r == nullptr) {
      return nullptr;
    }
    ++stats.hits;
    return &entry;
  }

  void clear() {
    if (stats.numEntries > 0) {
      for (auto& t : table) {
        std::fill(t.begin(), t.end(), Edge{});
      }
      stats.numEntries = 0;
    }
  }

private:
  std::size_t nvars;
  std::size_t numberOfOperations;
  std::vector<std::vector<Edge>> table;
  dd::TableStatistics stats{};
};

} // namespace dd::ddsim

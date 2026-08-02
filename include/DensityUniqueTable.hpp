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
 * @file DensityUniqueTable.hpp
 * @brief Unique table for density-matrix DD nodes.
 *
 * @details Self-contained copy of the relevant parts of the MQT Core
 * `dd::UniqueTable`, specialized for the density-matrix node type
 * `dd::ddsim::dNode`. Unlike the (post-removal) Core unique table, the node
 * equality check accounts for the persistent density-matrix flag, which is
 * required for the correctness of the reduced density-matrix representation.
 */

#pragma once

#include "DensityNode.hpp"
#include "dd/MemoryManager.hpp"
#include "dd/Node.hpp"
#include "dd/statistics/UniqueTableStatistics.hpp"
#include "ir/Definitions.hpp"

#include <cstddef>
#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

namespace dd::ddsim {

/// Data structure for uniquely storing density-matrix DD nodes.
class DensityUniqueTable {
public:
  static constexpr std::size_t INITIAL_GC_LIMIT = 131072U;

  struct UniqueTableConfig {
    std::size_t nVars = 0U;
    std::size_t nBuckets = 32768;
    std::size_t initialGCLimit = INITIAL_GC_LIMIT;
  };

  DensityUniqueTable(dd::MemoryManager& manager,
                     const UniqueTableConfig& config);

  void resize(std::size_t nVars);

  [[nodiscard]] std::size_t hash(const dNode& p) const {
    const std::size_t mask = cfg.nBuckets - 1;
    std::size_t key = 0U;
    for (const auto& succ : p.e) {
      qc::hashCombine(key, std::hash<dEdge>{}(succ));
    }
    key &= mask;
    return key;
  }

  [[nodiscard]] static bool nodesAreEqual(const dNode& p, const dNode& q) {
    return (p.e == q.e && (p.flags == q.flags));
  }

  // Lookup a node in the unique table and insert it if it has not been found.
  // Only normalized nodes shall be stored.
  [[nodiscard]] dNode* lookup(dNode* p) {
    // there are unique terminal nodes
    if (dd::NodeBase::isTerminal(p)) {
      return p;
    }

    const auto key = hash(*p);
    const auto v = p->v;
    ++stats[v].lookups;

    if (auto* hashedNode = searchTable(*p, key);
        !dNode::isTerminal(hashedNode)) {
      return hashedNode;
    }

    p->setNext(tables[v][key]);
    tables[v][key] = p;
    stats[v].trackInsert();

    return p;
  }

  [[nodiscard]] const auto& getTables() const { return tables; }
  [[nodiscard]] const auto& getStats() const noexcept { return stats; }
  [[nodiscard]] const dd::UniqueTableStatistics&
  getStats(std::size_t idx) const noexcept;
  [[nodiscard]] nlohmann::basic_json<>
  getStatsJson(bool includeIndividualTables = false) const;
  [[nodiscard]] std::size_t getNumEntries() const noexcept;
  [[nodiscard]] std::size_t countMarkedEntries() const noexcept;
  [[nodiscard]] bool possiblyNeedsCollection() const;
  std::size_t garbageCollect(bool force = false);
  void clear();

private:
  using Bucket = dd::NodeBase*;
  using Table = std::vector<Bucket>;

  UniqueTableConfig cfg;
  std::size_t gcLimit;
  dd::MemoryManager* memoryManager;
  std::vector<Table> tables;
  std::vector<dd::UniqueTableStatistics> stats;

  [[nodiscard]] dNode* searchTable(dNode& p, const std::size_t& key) {
    const auto v = p.v;
    auto* bucket = static_cast<dNode*>(tables[v][key]);
    while (bucket != nullptr) {
      if (nodesAreEqual(p, *bucket)) {
        // Match found
        if (&p != bucket) {
          // put node pointed to by p on available chain
          memoryManager->returnEntry(p);
        }
        ++stats[v].hits;
        return bucket;
      }
      ++stats[v].collisions;
      bucket = bucket->next();
    }

    // Node not found in bucket
    return dNode::getTerminal();
  }
};

} // namespace dd::ddsim

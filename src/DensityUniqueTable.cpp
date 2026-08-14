/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DensityUniqueTable.hpp"

#include "dd/MemoryManager.hpp"
#include "dd/Node.hpp"
#include "dd/statistics/UniqueTableStatistics.hpp"

#include <cstddef>
#include <numeric>

namespace dd::ddsim {

DensityUniqueTable::DensityUniqueTable(dd::MemoryManager& manager,
                                       const UniqueTableConfig& config)
    : cfg(config), gcLimit(config.initialGCLimit), memoryManager(&manager),
      tables(config.nVars, Table(config.nBuckets)), stats(config.nVars) {
  for (auto& stat : stats) {
    stat.entrySize = sizeof(Bucket);
    stat.numBuckets = cfg.nBuckets;
  }
}

void DensityUniqueTable::resize(const std::size_t nVars) {
  cfg.nVars = nVars;
  tables.resize(nVars, Table(cfg.nBuckets));
  stats.resize(nVars);
  for (auto& stat : stats) {
    stat.entrySize = sizeof(Bucket);
    stat.numBuckets = cfg.nBuckets;
  }
}

bool DensityUniqueTable::possiblyNeedsCollection() const {
  return getNumEntries() >= gcLimit;
}

std::size_t DensityUniqueTable::garbageCollect(const bool force) {
  const std::size_t numEntriesBefore = getNumEntries();
  if ((!force && numEntriesBefore < gcLimit) || numEntriesBefore == 0U) {
    return 0U;
  }

  std::size_t v = 0U;
  for (auto& table : tables) {
    auto& stat = stats[v];
    ++stat.gcRuns;
    for (auto& bucket : table) {
      dd::NodeBase* p = bucket;
      dd::NodeBase* lastp = nullptr;
      while (p != nullptr) {
        if (!p->isMarked()) {
          dd::NodeBase* next = p->next();
          if (lastp == nullptr) {
            bucket = next;
          } else {
            lastp->setNext(next);
          }
          memoryManager->returnEntry(*p);
          p = next;
          --stat.numEntries;
        } else {
          lastp = p;
          p = p->next();
        }
      }
    }
    ++v;
  }

  const auto numEntries = getNumEntries();
  if (numEntries > gcLimit / 10 * 9) {
    gcLimit = numEntries + cfg.initialGCLimit;
  }
  return numEntriesBefore - numEntries;
}

void DensityUniqueTable::clear() {
  for (auto& table : tables) {
    for (auto& bucket : table) {
      bucket = nullptr;
    }
  }
  gcLimit = cfg.initialGCLimit;
  for (auto& stat : stats) {
    stat.reset();
  }
}

const dd::UniqueTableStatistics&
DensityUniqueTable::getStats(const std::size_t idx) const noexcept {
  return stats.at(idx);
}

std::size_t DensityUniqueTable::getNumEntries() const noexcept {
  return std::accumulate(
      stats.begin(), stats.end(), std::size_t{0},
      [](const std::size_t& sum, const dd::UniqueTableStatistics& stat) {
        return sum + stat.numEntries;
      });
}

std::size_t DensityUniqueTable::countMarkedEntries() const noexcept {
  std::size_t count = 0U;
  for (const auto& table : tables) {
    for (auto* bucket : table) {
      auto* p = bucket;
      while (p != nullptr) {
        if (p->isMarked()) {
          ++count;
        }
        p = p->next();
      }
    }
  }
  return count;
}

} // namespace dd::ddsim

/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DDMinimizer.hpp"

#include "ir/Definitions.hpp"
#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/CompoundOperation.hpp"
#include "ir/operations/Control.hpp"
#include "ir/operations/OpType.hpp"
#include "ir/operations/Operation.hpp"

#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

namespace ddsim {
void DDMinimizer::optimizeInputPermutation(qc::QuantumComputation& circuit) {
  circuit.initialLayout = createGateBasedPermutation(circuit);
}

qc::Permutation
DDMinimizer::createGateBasedPermutation(const qc::QuantumComputation& circuit) {
  const auto bits = circuit.getNqubits();
  const auto isSet = [](const bool value) { return value; };
  if (bits < 2 || circuit.empty() ||
      std::ranges::any_of(circuit.getAncillary(), isSet) ||
      std::ranges::any_of(circuit.getGarbage(), isSet)) {
    return circuit.initialLayout;
  }

  std::vector<std::set<qc::Qubit>> targetToControls(bits);
  std::vector<std::size_t> remainingTargets(bits, 0);
  const auto collect = [&](const auto& self, const qc::Operation& op) -> void {
    if (const auto* compound =
            dynamic_cast<const qc::CompoundOperation*>(&op)) {
      for (const auto& child : *compound) {
        self(self, *child);
      }
    } else if (op.isStandardOperation() && op.getType() != qc::Z) {
      for (const auto& control : op.getControls()) {
        const auto c = circuit.initialLayout.at(control.qubit);
        for (const auto target : op.getTargets()) {
          const auto t = circuit.initialLayout.at(target);
          if (targetToControls.at(t).insert(c).second) {
            ++remainingTargets.at(c);
          }
        }
      }
    }
  };
  for (const auto& op : circuit) {
    collect(collect, *op);
  }

  std::set<qc::Qubit> ready;
  for (qc::Qubit qubit = 0; qubit < bits; ++qubit) {
    if (remainingTargets.at(qubit) == 0) {
      ready.insert(qubit);
    }
  }
  if (ready.size() == bits) {
    return circuit.initialLayout;
  }

  std::vector<qc::Qubit> position(bits);
  qc::Qubit next = 0;
  while (!ready.empty()) {
    const auto target = *ready.begin();
    ready.erase(ready.begin());
    position.at(target) = next++;
    for (const auto control : targetToControls.at(target)) {
      if (--remainingTargets.at(control) == 0) {
        ready.insert(control);
      }
    }
  }
  if (next != bits) {
    return circuit.initialLayout;
  }

  qc::Permutation permutation;
  for (const auto& [physical, logical] : circuit.initialLayout) {
    permutation[physical] = position.at(logical);
  }
  return permutation;
}
} // namespace ddsim

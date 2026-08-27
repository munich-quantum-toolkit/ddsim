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

#include "circuit_optimizer/CircuitOptimizer.hpp"
#include "ir/Definitions.hpp"
#include "ir/Permutation.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/Control.hpp"
#include "ir/operations/OpType.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace qc {

/***
 * Public Methods
 ***/

void DDMinimizer::optimizeInputPermutation(QuantumComputation& circuit) {
  const auto isSet = [](const bool value) { return value; };
  if (circuit.empty() || std::ranges::any_of(circuit.getAncillary(), isSet) ||
      std::ranges::any_of(circuit.getGarbage(), isSet)) {
    return;
  }

  // Normalize any existing physical-to-logical mapping before calculating a
  // new permutation on the circuit's logical qubits.
  CircuitOptimizer::elidePermutations(circuit);

  circuit.initialLayout = createGateBasedPermutation(circuit);
  CircuitOptimizer::elidePermutations(circuit);
}

Permutation
DDMinimizer::createGateBasedPermutation(const QuantumComputation& circuit) {
  // create the data structure to store the indices in the pattern maps as well
  // as the max indices of the ladders

  const std::size_t bits = circuit.getNqubits();
  const auto isSet = [](const bool value) { return value; };
  if (bits < 2 || std::ranges::any_of(circuit.getAncillary(), isSet) ||
      std::ranges::any_of(circuit.getGarbage(), isSet)) {
    return circuit.initialLayout;
  }

  // initialize the maps with the control and target qubits of the pattern
  DDMinimizer minimizer;
  minimizer.initializeDataStructure(bits);

  // iterate over all the ops and mark the index of the found x-c pairs in the
  // map.
  std::size_t instructionIndex = 0;
  bool found = false;
  for (const auto& op : circuit) {
    if (!op->isStandardOperation() || op->getType() == Z) {
      continue;
    }

    for (const auto& control : op->getControls()) {
      for (const auto& target : op->getTargets()) {
        auto itxC = minimizer.xCMap.find({control.qubit, target});
        if (itxC != minimizer.xCMap.end()) {
          itxC->second = instructionIndex;
          found = true;
        }
        auto itcX = minimizer.cXMap.find({control.qubit, target});
        if (itcX != minimizer.cXMap.end()) {
          itcX->second = instructionIndex;
          found = true;
        }
        for (std::size_t i = 0; i < bits - 1; i++) {
          auto itcL = minimizer.cLMap[i].find({control.qubit, target});
          if (itcL != minimizer.cLMap[i].end()) {
            itcL->second = instructionIndex;
            found = true;
          }
          auto itcH = minimizer.cHMap[i].find({control.qubit, target});
          if (itcH != minimizer.cHMap[i].end()) {
            itcH->second = instructionIndex;
            found = true;
          }
          auto itxL = minimizer.xLMap[i].find({control.qubit, target});
          if (itxL != minimizer.xLMap[i].end()) {
            itxL->second = instructionIndex;
            found = true;
          }
          auto itxH = minimizer.xHMap[i].find({control.qubit, target});
          if (itxH != minimizer.xHMap[i].end()) {
            itxH->second = instructionIndex;
            found = true;
          }
        }
      }
    }
    instructionIndex++;
  }
  if (!found) {
    return circuit.initialLayout;
  }

  // iterate over all the maps and find the max index of the found x-c pairs
  const InstructionIndex cXIndex = getCompletePatternEnd(minimizer.cXMap);
  const InstructionIndex xCIndex = getCompletePatternEnd(minimizer.xCMap);
  std::vector<InstructionIndex> cLIndex(bits - 1);
  std::vector<InstructionIndex> cHIndex(bits - 1);
  std::vector<InstructionIndex> xLIndex(bits - 1);
  std::vector<InstructionIndex> xHIndex(bits - 1);

  for (std::size_t i = 0; i < bits - 1; i++) {
    cLIndex[i] = getCompletePatternEnd(minimizer.cLMap[i]);
    cHIndex[i] = getCompletePatternEnd(minimizer.cHMap[i]);
    xLIndex[i] = getCompletePatternEnd(minimizer.xLMap[i]);
    xHIndex[i] = getCompletePatternEnd(minimizer.xHMap[i]);
  }

  // create the permutation based on the order of max index in the complete maps
  std::vector<Qubit> layout(bits);
  std::iota(layout.begin(), layout.end(), 0);

  const std::size_t prioCh = countPriorCompleteSteps(cHIndex, xCIndex);
  const std::size_t prioXl = countPriorCompleteSteps(xLIndex, xCIndex);
  const std::size_t stairsCh = getStairCount(cHIndex);
  const std::size_t stairsXl = getStairCount(xLIndex);
  const std::size_t prioCl = countPriorCompleteSteps(cLIndex, cXIndex);
  const std::size_t prioXh = countPriorCompleteSteps(xHIndex, cXIndex);
  const std::size_t stairsCl = getStairCount(cLIndex);
  const std::size_t stairsXh = getStairCount(xHIndex);

  // Check complete cases and adjust the layout.
  // reverse of  q | 0  1  2  3  turns to q | 0  1  2  3
  //             l | 0  1  2  3           l | 3  2  1  0

  if (cXIndex.has_value() && (!xCIndex.has_value() || *cXIndex < *xCIndex)) {
    std::reverse(layout.begin(), layout.end());
    if (prioCh == 0 && prioXl == 0) {
      if (stairsCh > 0) {
        layout = rotateRight(layout, stairsCh);
      } else if (stairsXl > 0) {
        layout = rotateLeft(layout, stairsXl);
      }
    }
  } else if (xCIndex.has_value() &&
             (!cXIndex.has_value() || *cXIndex > *xCIndex)) {
    if (prioCl == 0 && prioXh == 0) {
      if (isFullLadder(cLIndex) || isFullLadder(xHIndex)) {
        std::reverse(layout.begin(), layout.end());
      } else if (stairsCl > 0) {
        layout = rotateLeft(layout, stairsCl);
      } else if (stairsXh > 0) {
        layout = rotateRight(layout, stairsXh);
      }
    }
  } else if ((isFullLadder(xHIndex) || isFullLadder(cLIndex))) {
    std::reverse(layout.begin(), layout.end());
  } else {
    // in case no full pattern was identified, call fallback function to
    // determine ordering based on singular controlled operations
    return createControlBasedPermutation(circuit);
  }

  // transform layout into permutation
  // Permutation is std::map<Qubit, Qubit>
  Permutation perm;
  for (Qubit i = 0; i < bits; i++) {
    perm[i] = layout[i];
  }
  return perm;
}

void DDMinimizer::initializeDataStructure(std::size_t bits) {
  assert(bits >= 2);
  xCMap.clear();
  cXMap.clear();
  cLMap.resize(bits - 1);
  cHMap.resize(bits - 1);
  xLMap.resize(bits - 1);
  xHMap.resize(bits - 1);

  const std::size_t max = bits - 1;
  // create x-c ladder
  for (std::size_t i = 0; i < max; i++) {
    xCMap.insert({{i + 1, i}, std::nullopt});
    cXMap.insert({{i, i + 1}, std::nullopt});
  }

  // create c-l and x-l ladder
  for (std::size_t i = 0; i < max; i++) {
    for (std::size_t j = 0; j < bits; j++) {
      if (i < j) {
        xLMap[i].insert({{j, i}, std::nullopt});
        cLMap[i].insert({{i, j}, std::nullopt});
      }
    }
  }

  // create c-h and x-h ladder
  for (std::size_t i = max; i > 0; i--) {
    for (std::size_t j = 0; j < bits; j++) {
      if (i > j) {
        xHMap[bits - i - 1].insert({{j, i}, std::nullopt});
        cHMap[bits - i - 1].insert({{i, j}, std::nullopt});
      }
    }
  }
}

DDMinimizer::InstructionIndex
DDMinimizer::getCompletePatternEnd(const GatePattern& pattern) {
  InstructionIndex maxIndex;
  for (const auto& entry : pattern) {
    const InstructionIndex& index = entry.second;
    if (!index.has_value()) {
      return std::nullopt;
    }
    if (!maxIndex.has_value() || *index > *maxIndex) {
      maxIndex = index;
    }
  }
  return maxIndex;
}

// Helper function to check if the vector of a ladder step is full, meaning each
// gate appeared in the circuit
bool DDMinimizer::isFullLadder(const std::vector<InstructionIndex>& vec) {
  return std::ranges::all_of(
      vec, [](const InstructionIndex& value) { return value.has_value(); });
}

// Helper function to get the number of complete stairs in a ladder
std::size_t
DDMinimizer::getStairCount(const std::vector<InstructionIndex>& vec) {
  std::size_t count = 0;
  for (const auto& value : vec) {
    if (value.has_value()) {
      count++;
    } else {
      return count;
    }
  }
  return count;
}

// Count complete steps that occur before the competing adjacent ladder.
std::size_t
DDMinimizer::countPriorCompleteSteps(const std::vector<InstructionIndex>& steps,
                                     const InstructionIndex& ladderEnd) {
  if (!ladderEnd.has_value()) {
    return 0;
  }
  return static_cast<std::size_t>(
      std::ranges::count_if(steps, [&ladderEnd](const auto& stepEnd) {
        return stepEnd.has_value() && *stepEnd < *ladderEnd;
      }));
}

std::vector<Qubit> DDMinimizer::rotateLeft(std::vector<Qubit> layout,
                                           std::size_t stairs) {
  if (layout.empty()) {
    return layout;
  }
  stairs %= layout.size();
  std::rotate(layout.begin(),
              layout.begin() + static_cast<std::ptrdiff_t>(stairs),
              layout.end());
  return layout;
}

std::vector<Qubit> DDMinimizer::rotateRight(std::vector<Qubit> layout,
                                            std::size_t stairs) {
  if (layout.empty()) {
    return layout;
  }
  stairs %= layout.size();
  if (stairs != 0) {
    std::rotate(layout.begin(),
                layout.end() - static_cast<std::ptrdiff_t>(stairs),
                layout.end());
  }
  return layout;
}

// Fallback function to create a control based permutation if no pattern is
// found in the controlled gates
Permutation
DDMinimizer::createControlBasedPermutation(const QuantumComputation& circuit) {
  // create and fill a map of each qubit to all the qubits it controls
  std::map<Qubit, std::set<Qubit>> controlToTargets;

  // iterate over all the ops to mark which qubits are controlled by which
  // qubits
  for (const auto& op : circuit) {
    if (!op->isStandardOperation() || op->getType() == Z) {
      continue;
    }
    const Controls controls = op->getControls();
    const std::set<Qubit>& targets = {op->getTargets().begin(),
                                      op->getTargets().end()};

    for (const auto& control : controls) {
      if (controlToTargets.find(control.qubit) == controlToTargets.end()) {
        // If the control does not exist in the map, add it with the current set
        // of targetInts
        controlToTargets[control.qubit] = targets;
      } else {
        // If the control exists, insert the new targets into the existing set
        controlToTargets[control.qubit].insert(targets.begin(), targets.end());
      }
    }
  }

  if (controlToTargets.empty()) {
    return circuit.initialLayout;
  }

  const std::size_t bits = circuit.getNqubits();
  std::vector<std::size_t> remainingTargets(bits, 0);
  std::vector<std::set<Qubit>> targetToControls(bits);
  for (const auto& [control, targets] : controlToTargets) {
    if (control >= bits) {
      return circuit.initialLayout;
    }
    for (const auto target : targets) {
      if (target >= bits) {
        return circuit.initialLayout;
      }
      targetToControls[target].insert(control);
      ++remainingTargets[control];
    }
  }

  std::set<Qubit> ready;
  for (Qubit qubit = 0; qubit < bits; ++qubit) {
    if (remainingTargets[qubit] == 0) {
      ready.insert(qubit);
    }
  }

  std::vector<Qubit> layout;
  layout.reserve(bits);
  while (!ready.empty()) {
    const Qubit target = *ready.begin();
    ready.erase(ready.begin());
    layout.emplace_back(target);

    for (const auto control : targetToControls[target]) {
      --remainingTargets[control];
      if (remainingTargets[control] == 0) {
        ready.insert(control);
      }
    }
  }

  // A cycle has no ordering that places every target before its controllers.
  if (layout.size() != bits) {
    return circuit.initialLayout;
  }

  Permutation permutation;
  for (Qubit physical = 0; physical < bits; ++physical) {
    permutation[physical] = layout[physical];
  }
  return permutation;
}

} // namespace qc

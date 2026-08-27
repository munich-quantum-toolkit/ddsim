/*
 * Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
 * Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "DensityDDPackage.hpp"

#include "DensityNode.hpp"
#include "dd/Complex.hpp"
#include "dd/ComplexNumbers.hpp"
#include "dd/ComplexValue.hpp"
#include "dd/DDDefinitions.hpp"
#include "dd/GateMatrixDefinitions.hpp"
#include "dd/Node.hpp"
#include "dd/Package.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <vector>

namespace dd::ddsim {

///-----------------------------------------------------------------------------
///                          \n Node creation \n
///-----------------------------------------------------------------------------

dEdge DensityDDPackage::makeDDNode(const dd::Qubit var,
                                   const std::array<dEdge, dd::NEDGE>& edges,
                                   const bool generateDensityMatrix) {
  auto& mm = dMemoryManager;
  auto* p = mm.get<dNode>();
  p->v = var;
  p->flags = 0;
  p->setDensityMatrixNodeFlag(generateDensityMatrix);

  auto e = dEdge::normalize(p, edges, mm, pkg->cn);
  if (!e.isTerminal()) {
    const auto& es = e.p->e;
    // Check if node resembles the identity. If so, skip it.
    if ((es[0].p == es[3].p) &&
        (es[0].w.exactlyOne() && es[1].w.exactlyZero() &&
         es[2].w.exactlyZero() && es[3].w.exactlyOne())) {
      auto* ptr = es[0].p;
      mm.returnEntry(*e.p);
      return dEdge{.p = ptr, .w = e.w};
    }
  }

  auto* l = dUniqueTable.lookup(e.p);
  return dEdge{.p = l, .w = e.w};
}

dCachedEdge
DensityDDPackage::makeDDNode(const dd::Qubit var,
                             const std::array<dCachedEdge, dd::NEDGE>& edges,
                             const bool generateDensityMatrix) {
  auto& mm = dMemoryManager;
  auto* p = mm.get<dNode>();
  p->v = var;
  p->flags = 0;
  p->setDensityMatrixNodeFlag(generateDensityMatrix);

  auto e = dCachedEdge::normalize(p, edges, mm, pkg->cn);
  if (!e.isTerminal()) {
    const auto& es = e.p->e;
    if ((es[0].p == es[3].p) &&
        (es[0].w.exactlyOne() && es[1].w.exactlyZero() &&
         es[2].w.exactlyZero() && es[3].w.exactlyOne())) {
      auto* ptr = es[0].p;
      mm.returnEntry(*e.p);
      return dCachedEdge{ptr, e.w};
    }
  }

  auto* l = dUniqueTable.lookup(e.p);
  return dCachedEdge{l, e.w};
}

///-----------------------------------------------------------------------------
///                            \n Addition \n
///-----------------------------------------------------------------------------

dCachedEdge DensityDDPackage::add2(const dCachedEdge& x, const dCachedEdge& y,
                                   const dd::Qubit var) {
  if (x.w.exactlyZero()) {
    if (y.w.exactlyZero()) {
      return dCachedEdge::zero();
    }
    return y;
  }
  if (y.w.exactlyZero()) {
    return x;
  }
  if (x.p == y.p) {
    const auto rWeight = x.w + y.w;
    return {x.p, rWeight};
  }

  if (const auto* r = densityAdd.lookup(x, y); r != nullptr) {
    return *r;
  }

  constexpr std::size_t n = dd::NEDGE;
  std::array<dCachedEdge, n> edge{};
  for (std::size_t i = 0U; i < n; i++) {
    dCachedEdge e1{};
    if (x.isIdentity() || x.p->v < var) {
      // [ 0 | 1 ]   [ x | 0 ]
      // --------- = ---------
      // [ 2 | 3 ]   [ 0 | x ]
      if (i == 0 || i == 3) {
        e1 = x;
      }
    } else {
      auto& xSuccessor = x.p->e[i];
      e1 = {xSuccessor.p, 0};
      if (!xSuccessor.w.exactlyZero()) {
        e1.w = x.w * xSuccessor.w;
      }
    }
    dCachedEdge e2{};
    if (y.isIdentity() || y.p->v < var) {
      // [ 0 | 1 ]   [ y | 0 ]
      // --------- = ---------
      // [ 2 | 3 ]   [ 0 | y ]
      if (i == 0 || i == 3) {
        e2 = y;
      }
    } else {
      auto& ySuccessor = y.p->e[i];
      e2 = {ySuccessor.p, 0};
      if (!ySuccessor.w.exactlyZero()) {
        e2.w = y.w * ySuccessor.w;
      }
    }

    dNode::applyDmChangesToNode(e1.p);
    dNode::applyDmChangesToNode(e2.p);
    edge[i] = add2(e1, e2, var - 1);
    dNode::revertDmChangesToNode(e2.p);
    dNode::revertDmChangesToNode(e1.p);
  }
  auto r = makeDDNode(var, edge);
  densityAdd.insert(x, y, r);
  return r;
}

///-----------------------------------------------------------------------------
///                         \n Multiplication \n
///-----------------------------------------------------------------------------

dEdge DensityDDPackage::multiply(const dEdge& x, const dEdge& y,
                                 const bool generateDensityMatrix) {
  dd::Qubit var{};
  auto xCopy = x;
  auto yCopy = y;
  dEdge::applyDmChangesToEdges(xCopy, yCopy);

  if (!xCopy.isTerminal()) {
    var = xCopy.p->v;
  }
  if (!y.isTerminal() && yCopy.p->v > var) {
    var = yCopy.p->v;
  }

  const auto e = multiply2(xCopy, yCopy, var, generateDensityMatrix);
  dEdge::revertDmChangesToEdges(xCopy, yCopy);
  return dEdge{.p = e.p, .w = pkg->cn.lookup(e.w)};
}

dCachedEdge DensityDDPackage::multiply2(const dEdge& x, const dEdge& y,
                                        const dd::Qubit var,
                                        const bool generateDensityMatrix) {
  using ResultEdge = dCachedEdge;

  if (x.w.exactlyZero() || y.w.exactlyZero()) {
    return ResultEdge::zero();
  }

  const auto xWeight = static_cast<dd::ComplexValue>(x.w);
  const auto yWeight = static_cast<dd::ComplexValue>(y.w);
  const auto rWeight = xWeight * yWeight;
  if (x.isIdentity()) {
    if (y.isIdentity() ||
        (dNode::isDensityMatrixTempFlagSet(y.p->flags) &&
         generateDensityMatrix) ||
        (!dNode::isDensityMatrixTempFlagSet(y.p->flags) &&
         !generateDensityMatrix)) {
      return {y.p, rWeight};
    }
  }

  if (y.isIdentity()) {
    if (x.isIdentity() ||
        (dNode::isDensityMatrixTempFlagSet(x.p->flags) &&
         generateDensityMatrix) ||
        (!dNode::isDensityMatrixTempFlagSet(x.p->flags) &&
         !generateDensityMatrix)) {
      return {x.p, rWeight};
    }
  }

  if (const auto* r =
          densityDensityMultiplication.lookup(x.p, y.p, generateDensityMatrix);
      r != nullptr) {
    return {r->p, r->w * rWeight};
  }

  constexpr std::size_t n = dd::NEDGE;
  constexpr std::size_t rows = dd::RADIX;
  constexpr std::size_t cols = dd::RADIX;

  std::array<ResultEdge, n> edge{};
  for (auto i = 0U; i < rows; i++) {
    for (auto j = 0U; j < cols; j++) {
      auto idx = (cols * i) + j;
      edge[idx] = ResultEdge::zero();
      for (auto k = 0U; k < rows; k++) {
        const auto xIdx = (rows * i) + k;
        dEdge e1{};
        if (x.p != nullptr && x.p->v == var) {
          e1 = x.p->e[xIdx];
        } else {
          if (xIdx == 0 || xIdx == 3) {
            e1 = dEdge{.p = x.p, .w = dd::Complex::one()};
          } else {
            e1 = dEdge::zero();
          }
        }

        const auto yIdx = j + (cols * k);
        dEdge e2{};
        if (y.p != nullptr && y.p->v == var) {
          e2 = y.p->e[yIdx];
        } else {
          if (yIdx == 0 || yIdx == 3) {
            e2 = dEdge{.p = y.p, .w = dd::Complex::one()};
          } else {
            e2 = dEdge::zero();
          }
        }

        const auto v = static_cast<dd::Qubit>(var - 1);
        dCachedEdge m;
        dEdge::applyDmChangesToEdges(e1, e2);
        if (!generateDensityMatrix || idx == 1) {
          // When generateDensityMatrix is false or I have the first edge I
          // don't optimize anything and set generateDensityMatrix to false
          // for all child edges
          m = multiply2(e1, e2, v, false);
        } else if (idx == 2) {
          // When I have the second edge and generateDensityMatrix == false,
          // then edge[2] == edge[1]
          if (k == 0) {
            if (edge[1].w.approximatelyZero()) {
              edge[2] = ResultEdge::zero();
            } else {
              edge[2] = edge[1];
            }
          }
          continue;
        } else {
          m = multiply2(e1, e2, v, generateDensityMatrix);
        }

        if (k == 0 || edge[idx].w.exactlyZero()) {
          edge[idx] = m;
        } else if (!m.w.exactlyZero()) {
          dNode::applyDmChangesToNode(edge[idx].p);
          dNode::applyDmChangesToNode(m.p);
          edge[idx] = add2(edge[idx], m, v);
          dNode::revertDmChangesToNode(m.p);
          dNode::revertDmChangesToNode(edge[idx].p);
        }
        // Undo modifications on density matrices
        dEdge::revertDmChangesToEdges(e1, e2);
      }
    }
  }

  auto e = makeDDNode(var, edge, generateDensityMatrix);
  densityDensityMultiplication.insert(x.p, y.p, e);

  e.w = e.w * rWeight;
  return e;
}

///-----------------------------------------------------------------------------
///                             \n Trace \n
///-----------------------------------------------------------------------------

dd::ComplexValue DensityDDPackage::trace(const dEdge& a,
                                         const std::size_t numQubits) {
  if (a.isIdentity()) {
    return static_cast<dd::ComplexValue>(a.w);
  }
  const auto eliminate = std::vector<bool>(numQubits, true);
  return trace(a, eliminate, numQubits).w;
}

dCachedEdge DensityDDPackage::trace(const dEdge& a,
                                    const std::vector<bool>& eliminate,
                                    std::size_t level,
                                    std::size_t alreadyEliminated) {
  const auto aWeight = static_cast<dd::ComplexValue>(a.w);
  if (aWeight.approximatelyZero()) {
    return dCachedEdge::zero();
  }

  // If `a` is the identity matrix or there is nothing left to eliminate,
  // then simply return `a`
  if (a.isIdentity() ||
      std::none_of(eliminate.begin(),
                   eliminate.begin() +
                       static_cast<std::vector<bool>::difference_type>(level),
                   [](bool v) { return v; })) {
    return dCachedEdge{a.p, aWeight};
  }

  const auto v = a.p->v;
  if (eliminate[v]) {
    const auto eliminateAll =
        std::all_of(eliminate.begin(),
                    eliminate.begin() +
                        static_cast<std::vector<bool>::difference_type>(level),
                    [](bool e) { return e; });
    if (eliminateAll) {
      if (const auto* r = densityTrace.lookup(a.p); r != nullptr) {
        return {r->p, r->w * aWeight};
      }
    }

    const auto elims = alreadyEliminated + 1;
    auto r = add2(trace(a.p->e[0], eliminate, level - 1, elims),
                  trace(a.p->e[3], eliminate, level - 1, elims), v - 1);

    // Unlike for matrices, no normalization is applied for density matrices as
    // their trace is always 1 by definition.

    if (eliminateAll) {
      densityTrace.insert(a.p, r);
    }
    r.w = r.w * aWeight;
    return r;
  }

  std::array<dCachedEdge, dd::NEDGE> edge{};
  std::ranges::transform(a.p->e, edge.begin(),
                         [this, &eliminate, &alreadyEliminated,
                          &level](const dEdge& e) -> dCachedEdge {
                           return trace(e, eliminate, level - 1,
                                        alreadyEliminated);
                         });
  const auto adjustedV =
      static_cast<dd::Qubit>(static_cast<std::size_t>(a.p->v) -
                             (static_cast<std::size_t>(std::count(
                                  eliminate.begin(), eliminate.end(), true)) -
                              alreadyEliminated));
  auto r = makeDDNode(adjustedV, edge);
  r.w = r.w * aWeight;
  return r;
}

///-----------------------------------------------------------------------------
///                     \n High-level operations \n
///-----------------------------------------------------------------------------

dEdge DensityDDPackage::makeZeroDensityOperator(const std::size_t n) {
  auto f = dEdge::one();
  for (std::size_t p = 0; p < n; p++) {
    f = makeDDNode(static_cast<dd::Qubit>(p),
                   std::array{f, dEdge::zero(), dEdge::zero(), dEdge::zero()});
  }
  incRef(f);
  return f;
}

dEdge DensityDDPackage::applyOperationToDensity(dEdge& e,
                                                const dd::mEdge& operation) {
  const auto tmp0 = pkg->conjugateTranspose(operation);
  const auto tmp1 = multiply(e, dd::ddsim::densityFromMatrixEdge(tmp0), false);
  const auto tmp2 =
      multiply(dd::ddsim::densityFromMatrixEdge(operation), tmp1, true);
  incRef(tmp2);
  dEdge::alignDensityEdge(e);
  decRef(e);
  e = tmp2;
  dEdge::setDensityMatrixTrue(e);
  return e;
}

char DensityDDPackage::measureOneCollapsing(dEdge& e, const dd::Qubit index,
                                            std::mt19937_64& mt) {
  char measuredResult = '0';
  dEdge::alignDensityEdge(e);
  const auto nrQubits = e.p->v + 1U;
  dEdge::setDensityMatrixTrue(e);

  auto const measZeroDd = pkg->makeGateDD(dd::MEAS_ZERO_MAT, index);

  auto tmp0 = pkg->conjugateTranspose(measZeroDd);
  auto tmp1 = multiply(e, dd::ddsim::densityFromMatrixEdge(tmp0), false);
  auto tmp2 =
      multiply(dd::ddsim::densityFromMatrixEdge(measZeroDd), tmp1, true);
  auto densityMatrixTrace = trace(tmp2, nrQubits);

  std::uniform_real_distribution<dd::fp> dist(0., 1.);
  if (const auto threshold = dist(mt); threshold > densityMatrixTrace.r) {
    auto const measOneDd = pkg->makeGateDD(dd::MEAS_ONE_MAT, index);
    tmp0 = pkg->conjugateTranspose(measOneDd);
    tmp1 = multiply(e, dd::ddsim::densityFromMatrixEdge(tmp0), false);
    tmp2 = multiply(dd::ddsim::densityFromMatrixEdge(measOneDd), tmp1, true);
    measuredResult = '1';
    densityMatrixTrace = trace(tmp2, nrQubits);
  }

  dEdge::alignDensityEdge(e);
  tmp2.w = pkg->cn.lookup(e.w / densityMatrixTrace); // Normalize density matrix
  incRef(tmp2);
  decRef(e);
  e = tmp2;
  dEdge::setDensityMatrixTrue(e);

  return measuredResult;
}

///-----------------------------------------------------------------------------
///                    \n Reference counting and GC \n
///-----------------------------------------------------------------------------

void DensityDDPackage::incRef(const dEdge& e) {
  if (dEdge::trackingRequired(e)) {
    ++dRoots[e];
  }
  // Keep shared matrix nodes and complex numbers alive during the borrowed
  // package's own garbage collection by registering the (aligned) density DD
  // as a matrix root.
  pkg->incRef(matrixFromDensityEdge(e));
}

void DensityDDPackage::decRef(const dEdge& e) {
  if (dEdge::trackingRequired(e)) {
    if (auto it = dRoots.find(e); it != dRoots.end()) {
      if (--it->second == 0U) {
        dRoots.erase(it);
      }
    }
  }
  pkg->decRef(matrixFromDensityEdge(e));
}

bool DensityDDPackage::garbageCollect(const bool force) {
  if (!force && !dUniqueTable.possiblyNeedsCollection()) {
    return false;
  }
  for (const auto& edge : dRoots) {
    edge.first.mark();
  }
  const bool collected = dUniqueTable.garbageCollect(force) > 0;
  for (const auto& edge : dRoots) {
    edge.first.unmark();
  }
  if (collected) {
    densityAdd.clear();
    densityDensityMultiplication.clear();
    densityTrace.clear();
  }
  return collected;
}

std::size_t DensityDDPackage::computeActiveNodeCount() const {
  for (const auto& edge : dRoots) {
    edge.first.mark();
  }
  const auto count = dUniqueTable.countMarkedEntries();
  for (const auto& edge : dRoots) {
    edge.first.unmark();
  }
  return count;
}

} // namespace dd::ddsim

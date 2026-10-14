/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES.
 * All rights reserved. SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "cunls/common/export.h"
#include "factor_batch.h"

namespace cunls {

/**
 * @brief Compile-time sized base class for batched factors.
 *
 * This CRTP-style base class fixes the residual dimension and state
 * sizes at compile time via template arguments, enabling compile-time
 * optimizations in derived classes (e.g., fixed-size matrix operations).
 *
 * @tparam kResidualSize Dimension of the residual vector for each factor.
 * @tparam kStateSizes Variadic list of state dimensions.
 *         Each value specifies the number of scalar components in that state.
 *
 * @see FactorBatch for the polymorphic base interface.
 */
template <int kResidualSize, int... kStateSizes>
class SizedFactorBatch : public FactorBatch {
 public:
  /**
   * @brief Same SizedFactorBatch specialization as this base (for wrapper
   * types).
   *
   * Enables patterns like ``class Wrapper<T> : public T::sized_layout`` so
   * wrappers share the compile-time residual and state layout of T.
   */
  using sized_layout = SizedFactorBatch<kResidualSize, kStateSizes...>;

  /** @brief Batch without a capacity (subclasses that override NumActiveFactors()). */
  SizedFactorBatch() = default;

  /** @brief Batch whose buffers hold `capacity` factors; see FactorBatch(size_t). */
  explicit SizedFactorBatch(size_t capacity) : FactorBatch(capacity) {}

  /**
   * @brief Returns the compile-time residual dimension.
   * @return kResidualSize
   */
  size_t ResidualsSize() const final { return kResidualSize; };

  /**
   * @brief Returns the compile-time state sizes.
   * @return Vector containing {kStateSizes...}.
   */
  std::vector<size_t> StateSizes() const final { return {kStateSizes...}; };

  /** @brief Compile-time constant for the residual dimension. */
  static constexpr size_t residual_size_ = kResidualSize;
};
// Specializations used by exported cunls classes. MSVC exports a dllexport class's template
// base along with it, so every such base must be declared here too: otherwise a consumer that
// instantiates the same specialization collides with the DLL's copy (LNK2005).
extern template class CUNLS_API SizedFactorBatch<1, 1>;
extern template class CUNLS_API SizedFactorBatch<1, 1, 1>;
extern template class CUNLS_API SizedFactorBatch<1, 3>;
extern template class CUNLS_API SizedFactorBatch<1, 6>;
extern template class CUNLS_API SizedFactorBatch<2, 1, 1, 1, 1>;
extern template class CUNLS_API SizedFactorBatch<2, 6>;
extern template class CUNLS_API SizedFactorBatch<2, 6, 3>;
extern template class CUNLS_API SizedFactorBatch<3, 1, 1, 1, 1, 1, 1>;
extern template class CUNLS_API SizedFactorBatch<3, 3>;
extern template class CUNLS_API SizedFactorBatch<3, 3, 2, 3>;
extern template class CUNLS_API SizedFactorBatch<3, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<3, 3, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<3, 6>;
extern template class CUNLS_API SizedFactorBatch<4, 4>;
extern template class CUNLS_API SizedFactorBatch<4, 4, 4>;
extern template class CUNLS_API SizedFactorBatch<4, 6, 2, 6>;
extern template class CUNLS_API SizedFactorBatch<5, 3, 2, 2, 3, 2>;
extern template class CUNLS_API SizedFactorBatch<6, 3, 3, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<6, 6>;
extern template class CUNLS_API SizedFactorBatch<6, 6, 2, 2, 6, 2>;
extern template class CUNLS_API SizedFactorBatch<6, 6, 6>;
extern template class CUNLS_API SizedFactorBatch<6, 6, 6, 6>;
extern template class CUNLS_API SizedFactorBatch<7, 7>;
extern template class CUNLS_API SizedFactorBatch<7, 7, 7>;
extern template class CUNLS_API SizedFactorBatch<9, 3, 3, 3, 3, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<12, 6, 3, 3, 4, 6, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<12, 6, 3, 3, 12, 6, 3, 3>;
extern template class CUNLS_API SizedFactorBatch<12, 6, 6, 6, 6>;
extern template class CUNLS_API SizedFactorBatch<15, 6, 3, 6, 6, 3, 6>;
extern template class CUNLS_API SizedFactorBatch<15, 15>;
extern template class CUNLS_API SizedFactorBatch<15, 15, 15>;
extern template class CUNLS_API SizedFactorBatch<18, 6, 6, 6, 6, 6, 6>;

}  // namespace cunls

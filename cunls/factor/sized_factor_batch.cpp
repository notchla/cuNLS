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

#include "cunls/factor/sized_factor_batch.h"

namespace cunls {

template class SizedFactorBatch<1, 1>;
template class SizedFactorBatch<1, 1, 1>;
template class SizedFactorBatch<1, 3>;
template class SizedFactorBatch<1, 6>;
template class SizedFactorBatch<2, 1, 1, 1, 1>;
template class SizedFactorBatch<2, 6>;
template class SizedFactorBatch<2, 6, 3>;
template class SizedFactorBatch<3, 1, 1, 1, 1, 1, 1>;
template class SizedFactorBatch<3, 3>;
template class SizedFactorBatch<3, 3, 2, 3>;
template class SizedFactorBatch<3, 3, 3>;
template class SizedFactorBatch<3, 3, 3, 3>;
template class SizedFactorBatch<3, 6>;
template class SizedFactorBatch<4, 4>;
template class SizedFactorBatch<4, 4, 4>;
template class SizedFactorBatch<4, 6, 2, 6>;
template class SizedFactorBatch<5, 3, 2, 2, 3, 2>;
template class SizedFactorBatch<6, 3, 3, 3, 3>;
template class SizedFactorBatch<6, 6>;
template class SizedFactorBatch<6, 6, 2, 2, 6, 2>;
template class SizedFactorBatch<6, 6, 6>;
template class SizedFactorBatch<6, 6, 6, 6>;
template class SizedFactorBatch<7, 7>;
template class SizedFactorBatch<7, 7, 7>;
template class SizedFactorBatch<9, 3, 3, 3, 3, 3, 3>;
template class SizedFactorBatch<12, 6, 3, 3, 4, 6, 3, 3>;
template class SizedFactorBatch<12, 6, 3, 3, 12, 6, 3, 3>;
template class SizedFactorBatch<12, 6, 6, 6, 6>;
template class SizedFactorBatch<15, 6, 3, 6, 6, 3, 6>;
template class SizedFactorBatch<15, 15>;
template class SizedFactorBatch<15, 15, 15>;
template class SizedFactorBatch<18, 6, 6, 6, 6, 6, 6>;

}  // namespace cunls

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

// CUNLS_API marks every class or function whose definition lives in the library
// rather than in a header. CUNLS_BUILDING is set while compiling cunls itself;
// CUNLS_STATIC is propagated to consumers of the static library.
#if defined(_WIN32) && !defined(CUNLS_STATIC)
#if defined(CUNLS_BUILDING)
#define CUNLS_API __declspec(dllexport)
#else
#define CUNLS_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#define CUNLS_API __attribute__((visibility("default")))
#else
#define CUNLS_API
#endif

// CUNLS_CLASS_API marks a class exported member by member: its out-of-line members carry
// CUNLS_API. On MSVC the class stays unexported, because a dllexport class also exports its
// template base specialization (e.g. SizedStateBatch<16, 6>), which then collides (LNK2005)
// with a consumer's own instantiation. Elsewhere it exports the class's typeinfo and vtable.
#if defined(_WIN32)
#define CUNLS_CLASS_API
#else
#define CUNLS_CLASS_API CUNLS_API
#endif

// cuDSS classes are compiled only with CUNLS_ENABLE_CUDSS, and a dllexport class makes every
// includer emit its implicit members, which would not link without them.
#if defined(CUNLS_BUILDING) && !defined(CUNLS_ENABLE_CUDSS)
#define CUNLS_CUDSS_API
#else
#define CUNLS_CUDSS_API CUNLS_API
#endif

#ifdef _MSC_VER
// C4251/C4275 (and nvcc's 1394) flag std:: members and non-exported bases of
// exported classes. Benign: consumers must use the same MSVC toolset and
// Release CRT as cunls.
#pragma warning(disable : 4251 4275)
#ifdef __CUDACC__
#pragma nv_diag_suppress 1394
#endif
#endif

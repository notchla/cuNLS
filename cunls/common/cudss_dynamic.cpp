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

#include "cunls/common/cudss_dynamic.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdexcept>
#include <string>

#ifdef _WIN32
#define CUNLS_CUDSS_LIBRARY "cudss64_0.dll"
#define CUNLS_LIBRARY_PATH_VARIABLE "PATH"
#else
#define CUNLS_CUDSS_LIBRARY "libcudss.so"
#define CUNLS_LIBRARY_PATH_VARIABLE "LD_LIBRARY_PATH"
#endif

namespace cunls {

namespace {

#ifdef _WIN32
constexpr const char *kCudssLibraryCandidates[] = {"cudss64_0.dll"};
#else
constexpr const char *kCudssLibraryCandidates[] = {"libcudss.so.0", "libcudss.so"};
#endif

constexpr const char *kCudssMissingMessage =
    "cuNLS: the cuDSS-based sparse linear solver (SparseLinearSolverType::cuDSS) "
    "requires " CUNLS_CUDSS_LIBRARY
    ", which could not be found. cuDSS ships as a separate "
    "binary from cunls: download the matching cuDSS release from "
    "https://developer.download.nvidia.com/compute/cudss/redist/libcudss/ (or "
    "install your platform's cudss package), then add the directory containing " CUNLS_CUDSS_LIBRARY
    " to " CUNLS_LIBRARY_PATH_VARIABLE
    " before creating a cuDSS solver. Other cunls "
    "solvers (DenseLDLT, DenseCholesky, DenseQR, BlockSparsePCG) do not require "
    "cuDSS and are unaffected.";

#ifdef _WIN32
void *OpenLibrary(const char *name) {
  // The default-dirs search honours AddDllDirectory (Python's os.add_dll_directory)
  // but skips PATH, so fall back to the classic search order for PATH.
  HMODULE handle = LoadLibraryExA(name, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
  if (handle == nullptr) {
    handle = LoadLibraryA(name);
  }
  return handle;
}

void CloseLibrary(void *library) { FreeLibrary(static_cast<HMODULE>(library)); }

void *FindSymbol(void *library, const char *name, std::string &error) {
  void *symbol = reinterpret_cast<void *>(GetProcAddress(static_cast<HMODULE>(library), name));
  if (symbol == nullptr) {
    char buffer[256] = {};
    DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                  nullptr, GetLastError(), 0, buffer, sizeof(buffer), nullptr);
    while (length > 0 && (buffer[length - 1] == '\r' || buffer[length - 1] == '\n')) --length;
    error = length > 0 ? std::string(buffer, length) : "symbol not found";
  }
  return symbol;
}
#else
void *OpenLibrary(const char *name) { return dlopen(name, RTLD_NOW | RTLD_GLOBAL); }

void CloseLibrary(void *library) { dlclose(library); }

void *FindSymbol(void *library, const char *name, std::string &error) {
  dlerror();  // Clear any pending error.
  void *symbol = dlsym(library, name);
  const char *dlsym_error = dlerror();
  if (dlsym_error != nullptr || symbol == nullptr) {
    error = dlsym_error != nullptr ? dlsym_error : "symbol not found";
    return nullptr;
  }
  return symbol;
}
#endif

/**
 * @brief Resolves a cuDSS symbol, throwing a descriptive error if missing.
 *
 * A missing symbol (as opposed to a missing library) means a cuDSS library was
 * found but is a version incompatible with the cuDSS headers cunls was
 * built against.
 */
template <typename FnPtr>
FnPtr LoadCudssSymbol(void *library_handle, const char *symbol_name) {
  std::string error;
  void *symbol = FindSymbol(library_handle, symbol_name, error);
  if (symbol == nullptr) {
    throw std::runtime_error(
        std::string("cuNLS: found " CUNLS_CUDSS_LIBRARY " but it is missing the symbol '") +
        symbol_name +
        "'. The loaded cuDSS library is likely a version incompatible with the "
        "cuDSS headers cunls was built against. (" +
        error + ")");
  }
  return reinterpret_cast<FnPtr>(symbol);
}

/**
 * @brief Owns the library handle and the resolved cuDSS API table.
 */
struct CudssLoader {
  void *library_handle = nullptr;
  CudssApi api;
  bool available = false;
  std::string error_message;
};

void *OpenCudssLibrary() {
  for (const char *name : kCudssLibraryCandidates) {
    void *handle = OpenLibrary(name);
    if (handle != nullptr) {
      return handle;
    }
  }
  return nullptr;
}

/**
 * @brief Verifies the loaded cuDSS library reports the same MAJOR.MINOR
 * version as the cuDSS headers cunls was compiled against.
 *
 * A mismatched version is the same failure mode as a missing symbol (an ABI
 * cunls cannot safely call into) but may not always manifest as one, so it
 * is checked explicitly.
 */
void CheckCudssVersion(const CudssApi &api) {
  int runtime_major = 0;
  int runtime_minor = 0;
  if (api.GetProperty(MAJOR_VERSION, &runtime_major) != CUDSS_STATUS_SUCCESS ||
      api.GetProperty(MINOR_VERSION, &runtime_minor) != CUDSS_STATUS_SUCCESS) {
    throw std::runtime_error("cuNLS: found " CUNLS_CUDSS_LIBRARY
                             " but could not query its version via "
                             "cudssGetProperty(). The loaded cuDSS library is likely a version "
                             "incompatible with the cuDSS headers cunls was built against.");
  }

  if (runtime_major != CUDSS_VERSION_MAJOR || runtime_minor != CUDSS_VERSION_MINOR) {
    throw std::runtime_error(
        "cuNLS: found " CUNLS_CUDSS_LIBRARY " version " + std::to_string(runtime_major) + "." +
        std::to_string(runtime_minor) + ", but cunls was built against cuDSS " +
        std::to_string(CUDSS_VERSION_MAJOR) + "." + std::to_string(CUDSS_VERSION_MINOR) +
        " headers. Point " CUNLS_LIBRARY_PATH_VARIABLE " at a matching cuDSS " +
        std::to_string(CUDSS_VERSION_MAJOR) + "." + std::to_string(CUDSS_VERSION_MINOR) +
        ".x release instead.");
  }
}

CudssLoader LoadCudss() {
  CudssLoader loader;
  loader.library_handle = OpenCudssLibrary();
  if (loader.library_handle == nullptr) {
    loader.error_message = kCudssMissingMessage;
    return loader;
  }

  try {
    auto &api = loader.api;
    api.GetProperty =
        LoadCudssSymbol<decltype(api.GetProperty)>(loader.library_handle, "cudssGetProperty");
    api.Create = LoadCudssSymbol<decltype(api.Create)>(loader.library_handle, "cudssCreate");
    api.Destroy = LoadCudssSymbol<decltype(api.Destroy)>(loader.library_handle, "cudssDestroy");
    api.SetStream =
        LoadCudssSymbol<decltype(api.SetStream)>(loader.library_handle, "cudssSetStream");
    api.SetDeviceMemHandler = LoadCudssSymbol<decltype(api.SetDeviceMemHandler)>(
        loader.library_handle, "cudssSetDeviceMemHandler");
    api.SetThreadingLayer = LoadCudssSymbol<decltype(api.SetThreadingLayer)>(
        loader.library_handle, "cudssSetThreadingLayer");
    api.ConfigCreate =
        LoadCudssSymbol<decltype(api.ConfigCreate)>(loader.library_handle, "cudssConfigCreate");
    api.ConfigDestroy =
        LoadCudssSymbol<decltype(api.ConfigDestroy)>(loader.library_handle, "cudssConfigDestroy");
    api.ConfigSet =
        LoadCudssSymbol<decltype(api.ConfigSet)>(loader.library_handle, "cudssConfigSet");
    api.DataCreate =
        LoadCudssSymbol<decltype(api.DataCreate)>(loader.library_handle, "cudssDataCreate");
    api.DataDestroy =
        LoadCudssSymbol<decltype(api.DataDestroy)>(loader.library_handle, "cudssDataDestroy");
    api.MatrixCreateCsr = LoadCudssSymbol<decltype(api.MatrixCreateCsr)>(loader.library_handle,
                                                                         "cudssMatrixCreateCsr");
    api.MatrixCreateDn =
        LoadCudssSymbol<decltype(api.MatrixCreateDn)>(loader.library_handle, "cudssMatrixCreateDn");
    api.MatrixDestroy =
        LoadCudssSymbol<decltype(api.MatrixDestroy)>(loader.library_handle, "cudssMatrixDestroy");
    api.Execute = LoadCudssSymbol<decltype(api.Execute)>(loader.library_handle, "cudssExecute");

    CheckCudssVersion(api);

    loader.available = true;
  } catch (const std::exception &error) {
    loader.error_message = error.what();
    CloseLibrary(loader.library_handle);
    loader.library_handle = nullptr;
    loader.api = CudssApi{};
    loader.available = false;
  }

  return loader;
}

const CudssLoader &GetCudssLoader() {
  static const CudssLoader loader = LoadCudss();
  return loader;
}

}  // namespace

bool cuDSSIsAvailable() { return GetCudssLoader().available; }

const CudssApi &GetCudssApi() {
  const CudssLoader &loader = GetCudssLoader();
  if (!loader.available) {
    throw std::runtime_error(loader.error_message);
  }
  return loader.api;
}

}  // namespace cunls

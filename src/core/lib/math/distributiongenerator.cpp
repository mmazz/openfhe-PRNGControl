//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2014-2023, NJIT, Duality Technologies Inc. and other contributors
//
// All rights reserved.
//
// Author TPOC: contact@openfhe.org
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==================================================================================

/*
  This code provides basic structure for distribution generators. This should be inherited by
  all other distribution generators
 */

#include "math/distributiongenerator.h"
#include "utils/prng/blake2engine.h"
#include "utils/exception.h"
#include <cstdlib>      // getenv
#include <iostream>
#include <stdexcept>

#if (defined(__linux__) || defined(__unix__)) && !defined(__APPLE__) && defined(__GNUC__) && !defined(__clang__)
    #include <dlfcn.h>
#endif

namespace lbcrypto {

#if defined(WITH_OPENMP)
    std::shared_ptr<PRNG> PseudoRandomNumberGenerator::m_prng = nullptr;
#else
    thread_local std::shared_ptr<PRNG> m_prng = nullptr;
#endif
PseudoRandomNumberGenerator::GenPRNGEngineFuncPtr PseudoRandomNumberGenerator::genPRNGEngine = nullptr;

#if defined(WITH_OPENMP)
std::shared_ptr<PRNG> PseudoRandomNumberGenerator::m_prng = nullptr;
#if !defined(FIXED_SEED)
#pragma omp threadprivate(PseudoRandomNumberGenerator::m_prng)
#endif
#else
std::shared_ptr<PRNG> PseudoRandomNumberGenerator::m_prng = nullptr;
#endif

std::optional<uint64_t> PseudoRandomNumberGenerator::s_externalSeed = std::nullopt;

void PseudoRandomNumberGenerator::InitPRNGEngine(const std::string& libPath) {
    if (genPRNGEngine)  // if genPRNGEngine has already been initialized
        return;

    if (libPath.empty()) {
        // use the default OpenFHE PRNG that comes with the library
        genPRNGEngine = default_prng::createEngineInstance;
        if (!genPRNGEngine)
            OPENFHE_THROW("Cannot find symbol: default_prng::createEngineInstance");
        // std::cerr << "InitPRNGEngine: using local PRNG" << std::endl;
    }
    else {
#if (defined(__linux__) || defined(__unix__)) && !defined(__APPLE__) && defined(__GNUC__) && !defined(__clang__)
        // enable this code for g++ on Linux only
        // do not close libraryHandle, your application will crash if you do
        void* libraryHandle = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!libraryHandle) {
            std::string errMsg{std::string("Cannot open ") + libPath + ": "};
            const char* dlsym_error = dlerror();
            errMsg += dlsym_error;
            OPENFHE_THROW(errMsg);
        }
        genPRNGEngine = (GenPRNGEngineFuncPtr)dlsym(libraryHandle, "createEngineInstance");
        if (!genPRNGEngine) {
            std::string errMsg{std::string("Cannot load symbol createEngineInstance() from ") + libPath};
            const char* dlsym_error = dlerror();
            errMsg += ": ";
            errMsg += dlsym_error;
            dlclose(libraryHandle);
            OPENFHE_THROW(errMsg);
        }
        std::cerr << __FUNCTION__ << ": using external PRNG" << std::endl;
#else
        OPENFHE_THROW("OpenFHE may use an external PRNG library linked with g++ on Linux only");
#endif
    }
#ifndef FIXED_SEED
    if (!s_externalSeed) {
        if (const char* env = std::getenv("OPENFHE_SEED")) {
            try {
                s_externalSeed = std::stoull(env);
            } catch (...) {}
        }
    }
#endif
}
PRNG& PseudoRandomNumberGenerator::GetPRNG() {
    if (!m_prng) {
#pragma omp critical
        {
            // Asegurar inicialización del constructor
            if (!genPRNGEngine)
                InitPRNGEngine();
            if (!genPRNGEngine)
                throw std::runtime_error("Failure to initialize the PRNG engine");

#ifndef FIXED_SEED
            // Si hay semilla externa, crear Blake2Engine determinista
            if (s_externalSeed) {
                default_prng::Blake2Engine::blake2_seed_array_t arr{};
                arr[0] = *s_externalSeed;
                m_prng = std::make_shared<default_prng::Blake2Engine>(arr, 0);
            } else
#endif
            {
                // Motor por defecto (Blake2 con semilla fija o variable)
                m_prng = std::shared_ptr<PRNG>(genPRNGEngine());
            }

            if (!m_prng)
                throw std::runtime_error("Cannot create a PRNG engine");
        }
    }
    return *m_prng;
}
void PseudoRandomNumberGenerator::SetPRNGSeed(uint64_t seed) {
    s_externalSeed = seed;
    // Forzar reconstrucción en próxima GetPRNG()
    m_prng.reset();
}


}  // namespace lbcrypto

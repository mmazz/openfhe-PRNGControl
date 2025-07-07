//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2014-2022, NJIT, Duality Technologies Inc. and other contributors
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
  Simple examples for CKKS
 */

#define PROFILE

#include "openfhe.h"
using namespace lbcrypto;

int main() {
     uint64_t envSeed = 0;
    // Nombre de tu variable de entorno, p. ej. "OPENFHE_SEED"
    const char* envSeedStr = std::getenv("OPENFHE_SEED");
    if (envSeedStr) {
        try {
            envSeed = std::stoull(std::string(envSeedStr));
            std::cout << "Reading seed from enviroment variable: " << envSeed << "\n";
            // Ahora úsala en tu PRNG
            lbcrypto::PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: the value of OPENFHE_SEED is not valid: " << envSeedStr << "\n";
        }
    }
    else {
        std::cout << "OPENFHE_SEED was not correct defined, using default seed.\n";
    }


    uint32_t multDepth = 1;
    uint32_t scaleModSize = 50;
    uint32_t firstModSize = 60;
    uint32_t ringDim = 1<<8;
    uint32_t batchSize = ringDim >> 1;
    ScalingTechnique rescaleTech = FIXEDMANUAL;

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(multDepth);
    parameters.SetScalingModSize(scaleModSize);
    parameters.SetFirstModSize(firstModSize);
    parameters.SetBatchSize(batchSize);
    parameters.SetRingDim(ringDim);
    parameters.SetScalingTechnique(rescaleTech);
    parameters.SetSecurityLevel(HEStd_NotSet);


    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);

    cc->Enable(PKE);
    cc->Enable(LEVELEDSHE);
    std::cout << "CKKS scheme is using ring dimension " << cc->GetRingDimension() << std::endl << std::endl;

    auto keys = cc->KeyGen();

    std::vector<double> x1 = {0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 4.0, 5.0};

    Plaintext ptxt1 = cc->MakeCKKSPackedPlaintext(x1);

    std::cout << "Input x1: " << ptxt1 << std::endl;

    // Encrypt the encoded vectors
    PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
    auto c1 = cc->Encrypt(keys.publicKey, ptxt1);
    PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
    auto c2 = cc->Encrypt(keys.publicKey, ptxt1);
    size_t RNS_size = multDepth + 1;
    int count = 0 ;
    for (size_t i = 0; i < RNS_size; i++)
    {
        for(size_t k=0; k<2; k++)
        {
            for (size_t j = 0; j < ringDim; j++)
            {
                auto c1Elm = c1->GetElements()[k].GetAllElements()[i][j];
                auto c2Elm = c2->GetElements()[k].GetAllElements()[i][j];
                if(c1Elm!=c2Elm)
                    count++;
            }
        }
    }

    for (size_t j = 0; j < ringDim; j++)
    {
        auto c1Elm = c1->GetElements()[0].GetAllElements()[1][j];
        auto c2Elm = c2->GetElements()[0].GetAllElements()[1][j];
        std::cout << c1Elm << ", " << c2Elm << std::endl;
    }

    std::cout << "There were " << count << " different elements." << std::endl;
    Plaintext result;

    std::cout.precision(8);

    std::cout << std::endl << "Results of homomorphic computations: " << std::endl;

    cc->Decrypt(keys.secretKey, c1, &result);
    result->SetLength(batchSize);
    std::cout << "x1 = " << result;
    std::cout << "Estimated precision in bits: " << result->GetLogPrecision() << std::endl;



    return 0;
}

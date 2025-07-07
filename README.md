


# Install

'''
mkdir build
cd build
'''

To debug:
'''
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g -O0" ..
'''

To make campaigns without PIN:
'''
cmake -DCMAKE_BUILD_TYPE=Release -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF ..
'''

To use with PIN:
'''
cmake -DCMAKE_INSTALL_PREFIX=$HOME/ckksPin/openfhe-static -DBUILD_STATIC=ON \
      -DBUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g" ..
'''

After this, compile:
'''
make -j16
'''

### Posible bugs:

- Not installing cereal:
'''
cd third-party
git clone https://github.com/openfheorg/cereal
'''

## Changes:

Uncomment the flag FIXED_SEED at [blake2]{src/core/lib/utils/prng/blake2engine.cpp}.

We’ve extended the existing OpenFHE PRNG wrapper with three key additions to enable fully deterministic and resettable pseudo‑random streams:

1. **`static void SetPRNGSeed(uint64_t seed)`**
   - Forces the PRNG to use *exactly* the provided 64‑bit seed.
   - Resets the internal generator so that the *next* call to `GetPRNG()` will construct a fresh Blake2Engine with `m_counter = 0`.
   - Use this before any encryption or sampling call to guarantee reproducible noise sequences.

2. **`static std::optional<uint64_t> s_externalSeed`**
   - A new *static* holder for an optional external seed.
   - Populated either by `SetPRNGSeed(...)` or by reading the `OPENFHE_SEED` environment variable during initialization.
   - If present, supersedes the library’s default/fixed seed.

3. **Enhanced `InitPRNGEngine(const std::string& libPath = "")`**
   - After choosing the engine constructor (`default_prng::createEngineInstance` or a DLL lookup), it now checks for `OPENFHE_SEED` and stores its value in `s_externalSeed`.
   - Ensures environment‑driven seed configuration without touching code.

4. **Modified `GetPRNG()` logic**
   - Wraps the existing OpenMP‑safe initialization block.
   - On first call, if `s_externalSeed` is set, directly creates a `Blake2Engine` seeded with that value (counter = 0).
   - Otherwise falls back to the original `genPRNGEngine()` behavior (fixed seed or system randomness).
   - Subsequent calls return the same engine instance, letting its internal buffer and counter advance normally until you call `SetPRNGSeed(...)` again.

## Usage:

Before using it, export OPENFHE_SEED enviroment variable, or:

'''
OPENFHE_SEED=value ./bin/binary
'''

In your code, store the seed in some varible doing something like this:
'''
uint64_t envSeed = 0;
const char* envSeedStr = std::getenv("OPENFHE_SEED");
if (envSeedStr) {
    try {
        envSeed = std::stoull(std::string(envSeedStr));
        lbcrypto::PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
    }
    catch (...) {}
}
'''

And then reset the engine using:
'''
PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
'''

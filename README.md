


# Install

We prefer to install in a particular directory, if not do not put the flag
-DCMAKE_INSTALL_PREFIX.

'''
mkdir build
cd build
'''

To debug:
'''
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-prng/install -DBUILD_STATIC=OFF -DBUILD_SHARED=ON \
      -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g -O0" ..
'''

To make campaigns without PIN:
'''
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-prng/install -DBUILD_STATIC=OFF -DBUILD_SHARED=ON \
      -DCMAKE_BUILD_TYPE=Release -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF ..
'''

To use with PIN (static):
'''
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-prng/install -DBUILD_STATIC=ON -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g" ..
'''

After this, compile:
'''
make -j16
sudo make install
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


## 🔧 Configuration System

We added a lightweight configuration system to control fault injection and logging behavior via a single `config.json` file.

### ✅ Features

- Loads experiment parameters (e.g., injection mode, SDC threshold) from `config.json`.
- Automatically locates the file:
  - If `CKKS_CONFIG_PATH` is set, it uses that path.
  - Otherwise, defaults to `$HOME/ckksBitFlip/openfheBitFlip/config.json`.
- Centralized logging of SDC events into `SKA_crash.txt`.

### 🧪 Usage

```cpp
#include "config.hpp"

auto cfg = cfg::Config::Load();

if (cfg.injectError) {
    // Perform fault injection depending on cfg.injectMode
}

if (logstd > p - cfg.sdcThresholdBits) {
    cfg::logSDC(true);  // Log a Silent Data Corruption event
}
```

### 📄 Example `config.json`

```json
{
  "injectError": 1,         // Disable the inyection error with 0
  "injectMode": 3,          // 1: inyec only real part
                            // 2: inyect only imaginary part
                            // 3 or greater: inytect both
  "secretKeyAttack": 1,     // 0: enable execption
                            // 1 or greater: log the expection
  "sdcThresholdBits": 5     // amount of bits for threshold
}
```

This setup simplifies reproducibility and avoids scattered `.txt` flag files.

## Usage:

### Compilation of code


Use the same flags that the compilation of OpenFHE.

For thise we only care of the flags:

-  CMAKE_PREFIX_PATH = CMAKE_INSTALL_PREFIX
-  BUILD_STATIC
-  CMAKE_BUILD_TYPE
-  CMAKE_CXX_FLAGS

Ej:
'''
cmake -DCMAKE_PREFIX_PATH=$HOME/openfhe-prng/install -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O3" .
'''

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



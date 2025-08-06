# OpenFHE PRNG-Controlled Fork

This is a fork of the OpenFHE development branch.
It includes **minor changes to gain fine control over the PRNG engine**, using a **fixed seed by default**.
⚠️ **Not suitable for real-world use** — intended **only for research and experimentation**.

---

## 🔧 Installation

We recommend installing to a dedicated directory using `CMAKE_INSTALL_PREFIX`.
If you don’t specify it, OpenFHE installs system-wide.

```bash
mkdir build
cd build
```

### 🐞 Debug Build

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-PRNGControl/install -DBUILD_STATIC=OFF -DBUILD_SHARED=ON \
      -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g -O0" ..
```

### 🚀 Release Build (for campaigns **without** Intel PIN)

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-PRNGControl/install -DBUILD_STATIC=OFF -DBUILD_SHARED=ON \
      -DCMAKE_BUILD_TYPE=Release -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF ..
```

### 📦 Static Build (for use **with** Intel PIN)

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/openfhe-PRNGControl/install -DBUILD_STATIC=ON -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENMP=OFF -DBUILD_UNITTESTS=OFF \
      -DBUILD_BENCHMARKS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_CXX_FLAGS="-g" ..
```

### 🛠️ Compile and Install

```bash
make -j16
sudo make install
```

---

## 🐛 Common Issues

- **Missing Cereal dependency**:

```bash
cd third-party
git clone https://github.com/openfheorg/cereal
```

---

## ✨ Changes in this Fork

We added **deterministic PRNG control** by modifying `blake2engine.cpp`
→ Ensure the `FIXED_SEED` macro is **uncommented** in:

```
src/core/lib/utils/prng/blake2engine.cpp
```

### PRNG Enhancements:

1. **`SetPRNGSeed(uint64_t seed)`**
   - Forces the PRNG to start from the exact given seed.
   - Resets the internal Blake2Engine (`m_counter = 0`).
   - Use it to guarantee deterministic outputs (e.g., reproducible noise).

2. **`s_externalSeed` (static `std::optional<uint64_t>`)**
   - Holds a user-provided seed from `SetPRNGSeed(...)` or `OPENFHE_SEED`.
   - Overrides the default or fixed internal seed.

3. **Enhanced `InitPRNGEngine(...)`**
   - Initializes engine with `OPENFHE_SEED` if present.
   - Enables deterministic PRNG without code modifications.

4. **Modified `GetPRNG()` logic**
   - First-time initialization uses `s_externalSeed` if set.
   - Falls back to original behavior otherwise.
   - Can be reset at any time with `SetPRNGSeed(...)`.

---

## ⚙️ Fault Injection Configuration

We added a **minimal config system** for fault injection and logging.

### ✅ Features:

- Configurable via `config.conf`
- Location:
  - If `CKKS_CONFIG_PATH` is set → uses that path
  - Otherwise → defaults to: `$HOME/ckksBitFlip/openfheBitFlip/config.conf`
- Logs SDC events to `SKA_crash.txt`
- 
### 📄 Sample `config.conf`

```json
{
  "injectError": 1, 
  "injectMode": 3,
  "secretKeyAttackDisable ": 1, 
  "sdcThresholdBits": 5 
}
```
* injectError: If 0, no injection error
* injectMode: If injectError>0,
      * injectMode=1 is only to real
      * injectMode=2 is only to img
      * injectMode=3 is both (defualt of openfhe)
* secretKeyAttackDisable: If 1 we disable the secret key attack mechanism
* sdcThresholdBits: Amount of threshold, 5 is openfhe default
---
### 🧪 Usage Example

```cpp
#include "config.hpp"

auto cfg = cfg::Config::Load();

if (cfg.injectError) {
    // Apply fault injection based on cfg.injectMode
}

if (logstd > p - cfg.sdcThresholdBits) {
    cfg::logSDC(true);  // Log Silent Data Corruption (SDC) event
}
```



## 🛠️ Compiling Your Code

Use the same flags as OpenFHE's CMake build.
Essential ones:

- `CMAKE_PREFIX_PATH = <install-dir>/lib/OpenFHE`
- `BUILD_STATIC`
- `CMAKE_BUILD_TYPE`
- `CMAKE_CXX_FLAGS`

### Example:

```bash
cmake -DCMAKE_PREFIX_PATH=$HOME/openfhe-prng/install/lib/OpenFHE \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-g -O3" ..
```

---

## 🔁 Runtime PRNG Control

You can **set the PRNG seed** either via:

### Option 1: Environment Variable

```bash
export OPENFHE_SEED=12345
./bin/mybinary
```

Or inline:

```bash
OPENFHE_SEED=12345 ./bin/mybinary
```

### Option 2: In-Code Manual Seed Setting

```cpp
#include <cstdlib>

uint64_t envSeed = 0;
const char* envSeedStr = std::getenv("OPENFHE_SEED");

if (envSeedStr) {
    try {
        envSeed = std::stoull(std::string(envSeedStr));
        lbcrypto::PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
    } catch (...) {
        // Handle invalid input
    }
}

lbcrypto::PseudoRandomNumberGenerator::SetPRNGSeed(envSeed);
```

---

## 📘 License

Same license as [OpenFHE](https://github.com/openfheorg/openfhe-development).

---

## 🧪 Status

This fork is under active use in internal fault-injection research.
Expect occasional changes — **not stable for production**.

---

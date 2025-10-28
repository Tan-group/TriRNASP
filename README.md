# TriRNASP

**TriRNASP** — by *Tan-group, Wuhan University*  
*A knowledge-based three-body statistical potential for accurate RNA 3D structure evaluation.*

TriRNASP reads precomputed energy tables from the `Energy/` folder, processes `.pdb` structures, and outputs the lowest-energy candidates.

[![Download Test Sets](https://img.shields.io/badge/Download-Test%20Sets-blue)](https://github.com/Tan-group/TriRNASP/releases)

---

## 🔧 Build

### Using Make (Recommended)
```bash
make
```
Compiles **`TriRNASP`** with OpenMP optimization and extracts bundled datasets.

### Manual Build
```bash
gcc -O3 -march=native -ffast-math -fno-math-errno -fopenmp \
    -Wall -Wextra -Wa,--noexecstack -Wl,-z,noexecstack \
    TriRNASP.c -lm -o TriRNASP
```
**Requirements:** GCC with OpenMP, Linux/Unix.

---

## 📂 Inputs

- **Energy tables:** `Energy/Rough.energy`, `Energy/Fine.energy`
- **Structure folder:** contains `.pdb` RNA structures, e.g. `./example/`

Benchmark sets: [GitHub Releases](https://github.com/Tan-group/TriRNASP/releases)

---

## 🚀 Run

```bash
./TriRNASP <pdb_directory>
```
Example:
```bash
./TriRNASP ./example
```
**Output:**
```
Scanning directory: example/
Found PDBs: 182
R1205.pdb           -447.12
R1205TS481_2.pdb    -391.55
...
Wall-clock time: 0.81 s
```

---

## ⚙️ Config

In `TriRNASP.c`:
```c
#define num    1000  // Max PDBs per directory
#define path_l 300    // Max path length
```
When encountering stack-related core crashes, adjust `num` as needed to fit dataset size.

---

## 🨓 Clean

```bash
make clean
```

---

## 📩 Contact

**zjtan@whu.edu.cn**

---

## 🔖 Citation

```
Tongwei Yuan, En Lou, Zouchenyu Zhou, Ya-Lan Tan, Zhi-jie Tan.
TriRNASP: An efficient knowledge-based potential with three-body effect 
for accurate RNA 3D structure evaluation. (2025)
```

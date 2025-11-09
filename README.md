# 🧬 TriRNASP

**TriRNASP** — by *Tan-group, Wuhan University*  
*A knowledge-based three-body statistical potential for accurate RNA 3D structure evaluation.*

TriRNASP reads precomputed energy tables from the `Energy/` folder, processes `.pdb` structures, and outputs the lowest-energy candidates.  
The lower the energy (in *kBT*), the closer the structure is to the native state.

[![Download Test Sets](https://img.shields.io/badge/Download-Test%20Sets-blue)](https://github.com/Tan-group/TriRNASP/releases)

---

## 🧩 1. Quick Start

### 📥 Clone the Repository
```bash
git clone https://github.com/Tan-group/TriRNASP
cd TriRNASP
```

---

## 🔧 2. Build

### ✅ Recommended: Using Make
```bash
make
```
This will:
- Compile the executable **`TriRNASP`** with full OpenMP optimization.
- Automatically extract bundled datasets (`Energy.zip`, `Training_set.zip`, `exmaple.zip`).

After building, you should see the following structure:
```
TriRNASP/
 ├── TriRNASP           ← Executable file
 ├── Energy/            ← Energy tables
 ├── Training_set/      ← Training data
 └── exmaple/           ← Example dataset
```

### 🛠️ Manual Build (Alternative)
```bash
gcc -O3 -march=native -ffast-math -fno-math-errno -fopenmp \
    -Wall -Wextra -Wa,--noexecstack -Wl,-z,noexecstack \
    TriRNASP.c -lm -o TriRNASP
```
**Requirements:**  
- GCC with OpenMP support  
- Linux/Unix system

---

## 🚀 3. Run Example

Run TriRNASP on the provided example dataset:
```bash
./TriRNASP ./exmaple
```

**Example Output:**
```
Scanning  directory: ./exmaple/
Found PDBs: 182
R1205.pdb   -447.119919744277
R1205TS481_2.pdb   -391.548708213982
R1205TS481_1.pdb   -425.672444518381
R1205TS481_3.pdb   -292.657212955055
R1205TS481_4.pdb   -364.965577550650
Wall-clock time: 0.804421 seconds
```

✅ **Notes:**
- The output lists the **Top 5 lowest-energy structures** (in *kBT*).
- **Lower energy** indicates a structure **closer to the native state**.

---

## ⚙️ 4. Configuration

In `TriRNASP.c`:
```c
#define num    1000  // Max PDBs per directory
#define path_l 300   // Max path length
```
If a stack-related core crash occurs, adjust `num` to fit the dataset size.

---

## 🧼 5. Clean Build Files
```bash
make clean
```

---

## 🧠 Tips
- If you see a permission error:
  ```bash
  chmod +x TriRNASP
  ```
- If you get `command not found`, make sure you are in the TriRNASP directory.

---

## 📧 Contact
**Prof. Zhi-jie Tan**  
Wuhan University  
✉️ zjtan@whu.edu.cn

---

## 🔖 Citation
```
Tongwei Yuan, En Lou, Zouchenyu Zhou, Ya-Lan Tan, Zhi-jie Tan.
TriRNASP: An efficient knowledge-based potential with three-body effects
for accurate RNA 3D structure evaluation. (2025)
```

![header](img/WorkFlow.png)

# TriRNASP

**TriRNASP** — by *Tan-group, Wuhan University*  
*An efficient knowledge-based potential with three-body effect for accurate RNA 3D structure evaluation*

TriRNASP reads precomputed energy tables and atom type definitions from the `Energy/` folder, processes `.pdb` structures from a given directory, and outputs the lowest-energy candidates.

[![Download Test Sets](https://img.shields.io/badge/Download-Test%20Sets-blue)](https://github.com/Tan-group/TriRNASP/releases)

---

## 🔧 Build & Installation

### ✅ Recommended: Using Make

```bash
make
```

This will:
- Compile the executable **`TriRNASP`**
- Automatically extract bundled dataset archives (`*.zip`) if present

### 🛠️ Manual Build

```bash
gcc -O3 -fopenmp -Wall -Wextra \
    -Wa,--noexecstack -Wl,-z,noexecstack \
    TriRNASP.c -lm -o TriRNASP
```

**Requirements:**
- GCC with OpenMP support
- GNU Make
- Linux/Unix environment

---

## 📂 Input Files

Before running, ensure the following are available:

1. **Energy folder (`Energy/`)**
   - `Energy/Rough.energy`
   - `Energy/Fine.energy`

   These files provide statistical potential parameters.

2. **Structure directory**
   - A folder containing RNA structure files in `.pdb` format
   - Example: `./example/`

⚠️ **Note:** Official **TriRNASP test sets** (including benchmark PDBs and evaluation metrics such as DI, TM-score, and RMSD) are available from the **[GitHub Releases](https://github.com/Tan-group/TriRNASP/releases)** page.

---

## 🚀 Usage

Run TriRNASP on a directory of `.pdb` files:

```bash
./TriRNASP <structure_directory>
```

Example:

```bash
./TriRNASP ./example
```

The program will:
- Scan the directory for `.pdb` files
- Compute three-body energies for each structure
- Output the **top 5 lowest-energy structures**

---

## 📊 Example Output

```bash
./TriRNASP ./example
Scanning  directory: ./example/
Found PDBs: 182
R1205.pdb           -447.119919744277
R1205TS481_2.pdb    -391.548708213982
R1205TS481_1.pdb    -425.672444518381
R1205TS481_3.pdb    -292.657212955055
R1205TS481_4.pdb    -364.965577550650
Wall-clock time: 1.873877 seconds
```

### Example Error (insufficient stack size)

```bash
./TriRNASP R0251/
Scanning  directory: R0251/
Found PDBs: 23
Segmentation fault (core dumped)
```

This issue can be resolved by adjusting constants in the source code (see **Notes** below).

---

## 🧹 Clean Build

Remove object files and the executable:

```bash
make clean
```

---

## ⚡ Batch Processing with `batch.sh`

For large datasets containing many subfolders of PDB structures, a helper script **`batch.sh`** is included.

### Usage

```bash
./batch.sh
```

The script will:
- Traverse all subdirectories under `Test_sets/`
- Run **TriRNASP** on each directory
- Save energy results into `results_energy/` (one `.energy` file per directory)
- Record failed directories in `results_energy/failed_dirs.txt`
- Record PDB files that crash TriRNASP in `results_energy/crash_pdbs.txt`

### Parallel Execution

By default, `JOBS=1` (serial mode). To enable parallel processing, edit `batch.sh`:

```bash
JOBS=$(nproc)
```

This will use all available CPU cores for faster batch processing.

---

## ⚠️ Notes

- Input `.pdb` files must follow the standard **PDB format**.
- All required energy/data files must exist in the `Energy/` folder.
- Missing inputs will cause the program to terminate.
- OpenMP parallelization is enabled for **multi-core performance**.

### 🔧 Adjustable Parameters in Source Code
At the top of **`TriRNASP.c`**, two constants control memory usage and file path length:

```c
#define num     10000   // default = 10000
#define path_l  300     // default = 300
```

- **`num`** → Maximum number of `.pdb` structures allowed in a single directory.  
  - Default is 10000.  
  - If your dataset only contains a few hundred structures, this wastes memory and may cause **stack overflow / segmentation fault**.  
  - ✅ **Recommendation:** Reduce this value (e.g., 512 or 1024) according to dataset size.

- **`path_l`** → Maximum file path length (characters).  
  - Default is 300.  
  - Typically sufficient, since the Linux path length limit (`PATH_MAX`) is 4096.  
  - ⚠️ Do **not** set excessively large values, or memory usage will increase.

If you encounter crashes like:

```bash
Segmentation fault (core dumped)
```

then lowering `num` in `TriRNASP.c` and recompiling usually resolves the issue.

---

## 📬 Contact

For questions or feedback, please contact:  
📧 **zjtan@whu.edu.cn**

---

## 📖 Citation

If you use **TriRNASP** in your work, please cite:

```
Tovi Yuen, En Lou, Zouchenyu Zhou, Ya-Lan Tan, Zhi-jie Tan. 2025. TriRNASP: An efficient knowledge-based potential with three-body effect for accurate RNA 3D structure evaluation.
```

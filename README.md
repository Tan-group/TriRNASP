![header](img/WorkFlow.png)

# TriRNASP

**TriRNASP** — by *Tan-group, Wuhan University*  
A program for computing RNA **three-body statistical potentials** and **selecting structures closest to the native state**.  

It reads precomputed energy tables and atom type definitions from the `Energy/` folder, processes `.pdb` structures from a given directory, and outputs the lowest-energy candidates.

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
   - `Energy/12atom_type.dat`  

   These provide statistical potential parameters and the atom type list (12 entries).

2. **Structure directory**  
   - A folder containing RNA structure files in `.pdb` format  
   - Example: `./example/`

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

---

## 🧹 Clean Build

Remove object files and the executable:

```bash
make clean
```

---

## ⚠️ Notes

- Input `.pdb` files must follow the standard **PDB format**.  
- All required energy/data files must exist in the `Energy/` folder.  
- Missing inputs will cause the program to terminate.  
- OpenMP parallelization is enabled for **multi-core performance**.  

---

## 📬 Contact

For questions or feedback, please contact:  
📧 **zjtan@whu.edu.cn**

---

## 📖 Citation

If you use **TriRNASP** in your work, please cite:

```
Tovi Yuen, En Lou, Zouchenyu Zhou, Ya-Lan Tan, Zhi-jie Tan. 2025. TriRNASP. [Software].
Available at: https://github.com/Tan-group
```

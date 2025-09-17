![header](img/WorkFlow.png)
# TriRNASP

**Readme for TriRNASP package** by Tan-group at Wuhan University
**TriRNASP** is a program for computing RNA **three-body statistical potentials** and selecting structures closest to the native state.  
It reads precomputed energy tables and atom type definitions from the `Energy/` folder, processes PDB structures from a given directory, and outputs the lowest-energy candidates.

---

## Build

### Using Make (recommended)

```bash
make
```

This produces the executable `TriRNASP`.

### Manual build

```bash
gcc -O3 -fopenmp -Wall -Wextra -Wa,--noexecstack -Wl,-z,noexecstack TriRNASP.c -lm -o TriRNASP
```

**Requirements**:
- GCC with OpenMP support  
- GNU Make  
- Linux/Unix environment  

---

## Input Files

Before running, ensure the following are present:

1. **Energy folder (`Energy/`)**
   - `Energy/Rough.energy`  
   - `Energy/Fine.energy`  
   - `Energy/12atom_type.dat`  

   These files contain the statistical potential parameters and the atom type list (12 entries).

2. **Structure directory**  
   - A folder containing `.pdb` files, e.g. `./example/`.

---

## Usage

```bash
./TriRNASP <structure_directory>
```

Example:

```bash
./TriRNASP ./example
```

The program will:
- Scan the given directory for `.pdb` files  
- Compute three-body energies for each structure  
- Output the **top 5 lowest-energy structures**

---

## Example Output

```
Scanning  directory: ./example/
Found PDBs: 182
R1205.pdb           -447.119919744277
R1205TS481_2.pdb    -391.548708213982
R1205TS481_1.pdb    -425.672444518381
R1205TS481_3.pdb    -292.657212955055
R1205TS481_4.pdb    -364.965577550650
Wall-clock time: 2.181537 seconds
```

---

## Clean Build

```bash
make clean
```

Removes object files and the executable.

---

## Notes

- Input `.pdb` files must follow the standard PDB format.  
- All required data files (`Rough.energy`, `Fine.energy`, `12atom_type.dat`) must exist inside the `Energy/` folder.  
- Missing inputs will cause the program to terminate.  
- OpenMP parallelization is enabled for multi-core performance.  

## Contact

**If you have any questions about TriRNASP, please contact us by the email: zjtan@whu.edu.cn .**

## Citation Example

If you use **TriRNASP** in your work, please cite it as:

```
Tovi Yuen, En Lou, Zouchenyu Zhou, Ya-Lan Tan, Zhi-jie Tan. 2025. TriRNASP. [Software]. Available at: https://github.com/Tan-group
```

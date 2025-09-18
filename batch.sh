#!/usr/bin/env bash
set -euo pipefail

TRIRNASP="./TriRNASP"           # TriRNASP executable
TOP_DIR="./Test_sets"           # Top-level directory to scan
OUT_DIR="./results_energy"      # Output directory
JOBS=1                          # Parallelism (=1 serial; set to $(nproc) for all cores)

[[ -x "$TRIRNASP" ]] || { echo "Executable not found: $TRIRNASP"; exit 1; }
mkdir -p "$OUT_DIR"

FAILED_DIRS="$OUT_DIR/failed_dirs.txt"
CRASH_PDBS="$OUT_DIR/crash_pdbs.txt"
: > "$FAILED_DIRS"
: > "$CRASH_PDBS"

# Keep only lines like: "xxx.pdb <number>"
filter_lines() {
  awk '($1 ~ /\.pdb$/) && ($2 ~ /^-?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?$/) {print $1, $2}'
}

# Process a single directory: try once for the whole dir, then fallback per-PDB if needed
run_one() {
  local dir="$1"
  local rel outname outfile tmpout rc
  rel=$(realpath --relative-to="$TOP_DIR" "$dir" 2>/dev/null || echo "${dir#"$TOP_DIR"/}")
  outname="${rel//\//_}.energy"
  outfile="$OUT_DIR/$outname"

  echo ">>> Processing: $dir  ->  $outfile"

  # 1) Try running on the whole directory
  tmpout=$(mktemp)
  set +e
  "$TRIRNASP" "$dir" >"$tmpout" 2>>"$OUT_DIR/stderr.log"
  rc=$?
  set -e

  if [[ $rc -eq 0 ]]; then
    # Success: filter and write
    filter_lines < "$tmpout" > "$outfile"
    rm -f "$tmpout"
    return 0
  fi

  echo "[WARN] Whole-directory run failed, falling back to per-PDB: $dir (exit=$rc)" | tee -a "$FAILED_DIRS"

  # 2) Per-PDB fallback mode
  : > "$outfile"
  shopt -s nullglob
  local pdb
  for pdb in "$dir"/*.pdb; do
    # Create a temp dir per PDB because TriRNASP expects a directory
    local tmpdir
    tmpdir=$(mktemp -d)
    cp "$pdb" "$tmpdir/"
    local tmpout_p
    tmpout_p=$(mktemp)
    set +e
    "$TRIRNASP" "$tmpdir" >"$tmpout_p" 2>>"$OUT_DIR/stderr.log"
    rc=$?
    set -e
    if [[ $rc -eq 0 ]]; then
      filter_lines < "$tmpout_p" >> "$outfile"
    else
      echo "[CRASH] $pdb crashed TriRNASP (exit=$rc)" | tee -a "$CRASH_PDBS"
    fi
    rm -rf "$tmpdir" "$tmpout_p"
  done
  rm -f "$tmpout"
}

export -f run_one filter_lines
export TRIRNASP TOP_DIR OUT_DIR FAILED_DIRS CRASH_PDBS

# Find all directories that contain at least one .pdb
mapfile -d '' CANDIDATES < <(find "$TOP_DIR" -type d -print0 \
  | while IFS= read -r -d '' d; do
      compgen -G "$d/*.pdb" > /dev/null && printf '%s\0' "$d"
    done)

# Execute
if [[ "$JOBS" -le 1 ]]; then
  for d in "${CANDIDATES[@]}"; do
    run_one "$d"
  done
else
  printf '%s\0' "${CANDIDATES[@]}" | xargs -0 -n1 -P "$JOBS" bash -lc 'run_one "$0"'
fi

echo "Done. Results are in: $OUT_DIR/"
echo "- Failed directories: $FAILED_DIRS"
echo "- PDBs that crashed:  $CRASH_PDBS"


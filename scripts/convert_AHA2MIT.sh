#!/bin/bash

# ============================================================
# AHA to MIT-BIH Batch Converter
# ============================================================
# Description: Converts all .CMP files in the input directory
#              to WFDB format (.hea, .dat, .atr)
# ============================================================

# --- User Configuration ---
# Edit these paths to match your system:
SRC="/path/to/input/AHA/"          # Directory containing .CMP files
OUT="/path/to/output/for/save/result/AHA"  # Output directory

# --- Script Execution ---

# Create output directory if it doesn't exist
mkdir -p "$OUT"

# Change to source directory
cd "$SRC" || { echo "ERROR: Cannot access $SRC"; exit 1; }

# Convert each .CMP file
echo "Converting AHA files from: $SRC"
count=0

for f in *.CMP; do
    # Skip if no files match
    [ -e "$f" ] || { echo "No .CMP files found in $SRC"; exit 0; }
    
    echo "Processing: $f"
    ad2m -i "$f"
    
    if [ $? -eq 0 ]; then
        ((count++))
        echo "  ✓ Success"
    else
        echo "  ✗ Failed"
    fi
done

# Move all converted files to output directory
echo "Moving converted files to: $OUT"
mv *.atr *.dat *.hea "$OUT"/ 2>/dev/null

# Check if files were moved
if [ $? -eq 0 ]; then
    echo "=========================================="
    echo "Conversion Complete!"
    echo "Total records processed: $count"
    echo "Output location: $OUT"
    echo "=========================================="
else
    echo "Warning: No files were moved. Check if conversion succeeded."
fi
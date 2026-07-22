#!/bin/bash

# ============================================================
# AHA to MIT-BIH Batch Converter with Annotation Generation
# ============================================================

SRC="/home/amvaj-negar/Projects/DB/AHA Database Series 1/ECG Data"
OUT="/home/amvaj-negar/Projects/DB/AHA Database Series 1/ECG Data/AHA"

# --- Conversion ---
mkdir -p "$OUT"
cd "$SRC" || { echo "ERROR: Cannot access $SRC"; exit 1; }

echo "Converting AHA files from: $SRC"
count=0

for f in *.CMP; do
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

for f in *.ANO; do
    [ -e "$f" ] || { echo "No .ANO files found in $SRC"; exit 0; }
    
    echo "Processing: $f"
    a2m -i "$f"
    
    if [ $? -eq 0 ]; then
        ((count++))
        echo "  ✓ Success"
    else
        echo "  ✗ Failed"
    fi
done

# Move files
echo "Moving converted files to: $OUT"
mv *.atr *.dat *.hea "$OUT"/ 2>/dev/null



echo "=========================================="
echo "Conversion Complete!"
echo "Total records processed: $count"
echo "Output location: $OUT"
echo "=========================================="

# Show sample
echo ""
echo "Sample of generated files:"
ls -lh *.hea *.dat *.atr 2>/dev/null | head -10
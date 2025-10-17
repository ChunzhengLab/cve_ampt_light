#!/bin/bash

# Timestamp for bookkeeping
echo "=== JOB STARTED [$(date)] ==="

# Load Conda environment and ROOT
echo "Activating conda environment cpp_dev"
source /storage/fdunphome/wangchunzheng/miniconda3/etc/profile.d/conda.sh
conda activate cpp_dev

# Set compiler wrappers and clean ROOT environment
export CC="$CONDA_PREFIX/bin/x86_64-conda-linux-gnu-cc"
export CXX="$CONDA_PREFIX/bin/x86_64-conda-linux-gnu-c++"
unset ROOTSYS

# Remove conflicting ROOT paths
export PATH=$(echo ":$PATH:" | sed -E 's#:/opt/root61404/bin:#:#g' | sed 's#^:##;s#:$##')
export LD_LIBRARY_PATH=$(echo ":$LD_LIBRARY_PATH:" | sed -E 's#:/opt/root61404/lib:#:#g' | sed 's#^:##;s#:$##')

INPUT_FILE=$1
JOB_ID=$2
OUTPUT_DIR="/storage/fdunphome/wangchunzheng/cve_ampt_light/afterART/outputs"
mkdir -p "${OUTPUT_DIR}"

# Run the analysis executable for this list
echo "Running: /storage/fdunphome/wangchunzheng/cve_ampt_light/analysis_cve $INPUT_FILE $OUTPUT_DIR/results_${JOB_ID}.root"
/storage/fdunphome/wangchunzheng/cve_ampt_light/analysis_cve "$INPUT_FILE" "$OUTPUT_DIR/results_${JOB_ID}.root"

# Deactivate environment and finish
echo "=== JOB COMPLETED [$(date)] ==="
conda deactivate
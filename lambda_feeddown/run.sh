#!/bin/bash

# Timestamp for bookkeeping
echo "=== Lambda Feeddown Analysis JOB STARTED [$(date)] ==="

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
OUTPUT_DIR="/storage/fdunphome/wangchunzheng/cve_ampt_light/lambda_feeddown/outputs"
mkdir -p "${OUTPUT_DIR}"

# Print job information
echo "Job ID: ${JOB_ID}"
echo "Input file: ${INPUT_FILE}"
echo "Output directory: ${OUTPUT_DIR}"

# Run the Lambda feeddown analysis executable
EXECUTABLE="/storage/fdunphome/wangchunzheng/cve_ampt_light/analysis_lambda_feeddown"
OUTPUT_FILE="${OUTPUT_DIR}/feeddown_results_${JOB_ID}.root"

echo "Running: ${EXECUTABLE} ${INPUT_FILE} ${OUTPUT_FILE}"
${EXECUTABLE} "${INPUT_FILE}" "${OUTPUT_FILE}"

EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    echo "Analysis completed successfully!"
    echo "Output saved to: ${OUTPUT_FILE}"
else
    echo "ERROR: Analysis failed with exit code ${EXIT_CODE}"
fi

# Deactivate environment and finish
echo "=== Lambda Feeddown Analysis JOB COMPLETED [$(date)] ==="
conda deactivate

exit $EXIT_CODE

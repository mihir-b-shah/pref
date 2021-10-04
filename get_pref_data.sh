#!/bin/bash

CHAMPSIM_DIR="champsim"
PREFETCHERS="bo"
DATA_NAME="pref_track_data"
OUT_DIR="../reports"
BUILD_FILE="build_output"
BENCHMARK_PATH=$(basename $1)
PREV_DIR=$(pwd)

cd ${CHAMPSIM_DIR}

for pref in "$PREFETCHERS"; do
  ./build_champsim.sh bimodal no no ${pref} no lru "1" &> ${BUILD_FILE}
  if [ $? -eq 0 ]; then
    ./run_champsim.sh bimodal-no-no-${pref}-no-lru-1core 1 10 ${BENCHMARK_PATH}
    cp ${DATA_NAME} ${OUT_DIR}/${pref}_report
    rm ${DATA_NAME} ${BUILD_FILE}
    echo "Completed for prefetecher ${pref}"
  else
    echo "Unable to build champsim on prefetcher ${pref}, check ${BUILD_FILE}"
    break
  fi
done

cd ${PREV_DIR}

#!/bin/bash

genTempl(){
    init_dir="$1"
    exe="$2"
    shift 2

    echo "
#!/bin/bash

+Group=\"UNDER\"
+Project=\"ARCHITECTURE\"
+ProjectDescription=\"Architectural Simulation\"

universe=vanilla
getenv=true
Rank=Memory
notification=Error
notify_user=mihirs@cs.utexas.edu
error=mihirs.CONDOR.ERR
output=mihirs.CONDOR.OUT
initial_dir=$init_dir
executable=$exe
arguments=${@}

queue
    "
}

CHAMPSIM_DIR="champsim"
PREFETCHERS="ip_stride" #"bo no sisb sms ip_stride"
BUILD_FILE="build_output"
BENCHMARK_PATH=$(basename $1)
PREV_DIR=$(pwd)

cd ${CHAMPSIM_DIR}

for pref in $PREFETCHERS; do
  ./build_champsim.sh bimodal no no ${pref} no lru "1" &> ${BUILD_FILE}
  if [ $? -eq 0 ]; then
    RUN_CMD="run_champsim.sh bimodal-no-no-${pref}-no-lru-1core 1 250 ${BENCHMARK_PATH}"
    JOB_FILE="job_file_$pref"

    genTempl "." ${RUN_CMD} >${JOB_FILE}
    condor_submit ${JOB_FILE}

    echo "Completed for prefetcher ${pref}"
  else
    echo "Unable to build champsim on prefetcher ${pref}, check ${BUILD_FILE}"
    break
  fi
done

cd ${PREV_DIR}

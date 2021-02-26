#!/bin/bash

# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Benchmarks modules generated by compile_modules.sh on Android and writes
# performance data to a text proto file, mako-SERIAL_NUMBER-git_hash.
#
# The script benchmarks modules on 7-th big core, ie, run with `taskset 80`.

set -e
set -o pipefail
set -o xtrace

git_hash=UNKNOWN
while [[ $# -gt 0 ]]; do
  token="$1"
  case $token in
    --model=*)
      model="${1#*=}"
      shift
      ;;
    --targets=*)
      targets="${1#*=}"
      shift
      ;;
    --benchmark_key=*)
      benchmark_key="${1#*=}"
      shift
      ;;
    --git_hash=*)
      git_hash="${1#*=}"
      shift
      ;;
  esac
done

TS="$(date +%s)000"
SED_EXPR='s/BM_[[:alnum:][:punct:]]+\/real_time[[:space:]]+ ([.0-9]+) ms[[:print:]]+/\1/p'
OUTPUT_DIR=test_output_files
DEVICE_ROOT=/data/local/tmp/benchmark_tmpdir

mkdir -p "${OUTPUT_DIR}"

if [[ -z "${model}" ]]; then
  echo "Must set --model flag.";
  exit 1
fi

if [[ -z "${benchmark_key}" ]]; then
  echo "Must set --benchmark_key flag.";
  exit 1
fi

function append_mako_metadata {
  local mako_log="$1"
  cat >> "${mako_log}" << EOF
metadata: {
  git_hash: "${git_hash}"
  timestamp_ms: ${TS}
  benchmark_key: "${benchmark_key}"
}
EOF
}

function append_mako_sample {
  local mako_log="$1"
  local value="$2"
  local tag="$3"
  cat >> "${mako_log}" << EOF
samples: {
  time: ${value}
  target: "${tag}"
}
EOF
}

function run_and_log {
  local mako_log="$1"
  local target="$2"
  extra_flags=()
  case "${target}" in
    "vulkan-spirv")
      TAG='vlk'
      ;;
    "vmla")
      TAG='vmla'
      ;;
    "dylib-llvm-aot")
      TAG='cpu'
      # TODO(hanchung): Figure out a better way to customize extra flags for
      # different benchmarking targets. This forces all dylib targets use single
      # thread.
      extra_flags+=('--dylib_worker_count=1')
      ;;
    *)
      echo "Unrecognized target '${target}'"
      exit 1
      ;;
  esac

  driver="$(echo ${target} | cut -d '-' -f1)"
  test_out="${OUTPUT_DIR}/${model}-${target}_output.txt"
  adb shell LD_LIBRARY_PATH=/data/local/tmp taskset 80 \
    "${DEVICE_ROOT}/iree-benchmark-module" \
    "--flagfile=${DEVICE_ROOT}/flagfile" \
    "--module_file=${DEVICE_ROOT}/${model}-${target}.vmfb" \
    "--driver=${driver}" \
    "${extra_flags[@]}" \
    --benchmark_repetitions=10 | tee "${test_out}"
  while read -r ms; do
    append_mako_sample "${mako_log}" "${ms}" "${TAG}"
  done < <(sed -En -e "${SED_EXPR}" "${test_out}")
}

phone_serial_number="$(adb get-serialno)"
mako_log="mako-${phone_serial_number}-${git_hash}.log"
: > "${mako_log}" # Empty the log file

IFS=',' read -ra targets_array <<< "$targets"
for target in "${targets_array[@]}"
do
  echo "Executing ${target}..."
  run_and_log "${mako_log}" "${target}"
done

append_mako_metadata "${mako_log}"
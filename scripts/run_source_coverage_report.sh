#!/bin/bash
set -o pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  cat <<'USAGE'
usage: scripts/run_source_coverage_report.sh [options]

Build cppgm with Clang source coverage, run the assignment test report plus
supplemental strict/template-kernel tests, merge profiles, and write llvm-cov
reports.

Options:
  --root DIR             Coverage output root (default: /tmp/cppgm-source-coverage-<timestamp>)
  --pas "pa1 pa2"        Assignment list for ACTIVE_TEST_REPORT_PAS
  --by-pa                Also generate per-assignment coverage JSON and attribution
  --jobs N              Build make jobs (default: 4)
  --subtest-jobs N      TEST_REPORT_SUBTEST_JOBS (default: 2)
  --assignment-jobs N   TEST_REPORT_ASSIGNMENT_JOBS override
  --strict-pas "pa18"   Strict supplemental PA list (default: pa19 pa20 pa22 pa23 pa24)
  --skip-strict         Skip strict semantic/witness supplemental coverage
  --skip-template-kernel
                         Skip template-kernel supplemental coverage
  --skip-memory-census  Skip CPPGM_MEMORY_CENSUS supplemental coverage
  --skip-linux-target   Skip Linux-target object/executable supplemental coverage
  --skip-host-runtime   Skip host builtin runtime supplemental coverage
  --skip-host-eh-object
                         Skip host EH object/roundtrip supplemental coverage
  --skip-semantic-hotspot
                         Skip CPPGM_SEMANTIC_HOTSPOT supplemental coverage
  --skip-diagnostic-instrumentation
                         Skip diagnostic env instrumentation supplemental coverage
  --skip-cli-batch       Skip non-test-runner cli batch frontend supplemental coverage
  --skip-tool-help      Skip tool help/query supplemental coverage
  --cxx PATH            Clang++ path
  --host-cxx PATH       CPPGM_HOST_CXX path
  --skip-build          Reuse existing coverage build in --root
  --skip-tests          Reuse existing profraw data in --root
  --no-html             Skip llvm-cov HTML generation
  --help                Show this help

Environment equivalents:
  COV_ROOT, ACTIVE_TEST_REPORT_PAS, COVERAGE_JOBS,
  TEST_REPORT_SUBTEST_JOBS, TEST_REPORT_ASSIGNMENT_JOBS, CXX, CPPGM_HOST_CXX,
  COVERAGE_BY_PA, COVERAGE_STRICT, COVERAGE_STRICT_PAS,
  COVERAGE_TEMPLATE_KERNEL, COVERAGE_MEMORY_CENSUS, COVERAGE_LINUX_TARGET,
  COVERAGE_HOST_RUNTIME, COVERAGE_HOST_EH_OBJECT, COVERAGE_SEMANTIC_HOTSPOT,
  COVERAGE_DIAGNOSTIC_INSTRUMENTATION, COVERAGE_CLI_BATCH,
  COVERAGE_TOOL_HELP,
  COVERAGE_IGNORE_REGEX
USAGE
}

find_llvm_root() {
  if [ -n "${LLVM_ROOT:-}" ] && [ -x "${LLVM_ROOT}/bin/llvm-cov" ]; then
    printf '%s\n' "${LLVM_ROOT}"
    return 0
  fi
  if [ -x /usr/local/opt/llvm/bin/llvm-cov ]; then
    printf '%s\n' /usr/local/opt/llvm
    return 0
  fi
  if [ -x /opt/homebrew/opt/llvm/bin/llvm-cov ]; then
    printf '%s\n' /opt/homebrew/opt/llvm
    return 0
  fi
  return 1
}

selected_pas() {
  if [ -n "${active_pas}" ]; then
    for pa in ${active_pas}; do
      printf '%s\n' "${pa}"
    done
    return 0
  fi
  for n in $(seq 1 99); do
    if [ -d "${repo_root}/pa${n}" ]; then
      printf 'pa%s\n' "${n}"
    fi
  done
}

write_coverage_objects_list() {
  local output="$1"
  local stamp="$2"
  local path
  local pa_dir
  local tool

  : > "${output}"
  for tool in "${tool_names[@]}"; do
    path="${repo_root}/dev/${tool}"
    if [ -x "${path}" ]; then
      printf '%s\n' "${path}" >> "${output}"
    fi
  done
  if [ -f "${extra_coverage_objects:-}" ]; then
    cat "${extra_coverage_objects}" >> "${output}"
  fi
  for pa_dir in "${repo_root}"/pa[0-9]*; do
    [ -d "${pa_dir}" ] || continue
    for tool in "${tool_names[@]}"; do
      path="${pa_dir}/${tool}"
      if [ -x "${path}" ] && { [ -z "${stamp}" ] || [ "${path}" -nt "${stamp}" ]; }; then
        printf '%s\n' "${path}" >> "${output}"
      fi
    done
  done
  sort -u -o "${output}" "${output}"
}

write_pa_coverage_objects_list() {
  local output="$1"
  local pa="$2"
  local path
  local tool

  : > "${output}"
  for tool in "${tool_names[@]}"; do
    path="${repo_root}/dev/${tool}"
    if [ -x "${path}" ]; then
      printf '%s\n' "${path}" >> "${output}"
    fi
  done
  for tool in "${tool_names[@]}"; do
    path="${repo_root}/${pa}/${tool}"
    if [ -x "${path}" ]; then
      printf '%s\n' "${path}" >> "${output}"
    fi
  done
  sort -u -o "${output}" "${output}"
}

remove_one_pa_tool_binaries() {
  local pa="$1"
  local path
  local tool

  for tool in "${tool_names[@]}"; do
    path="${repo_root}/${pa}/${tool}"
    rm -f "${path}"
    rm -rf "${path}.dSYM"
  done
}

remove_selected_pa_tool_binaries() {
  local pa

  while IFS= read -r pa; do
    [ -n "${pa}" ] || continue
    [ -d "${repo_root}/${pa}" ] || continue
    remove_one_pa_tool_binaries "${pa}"
  done <<EOF
$(selected_pas)
EOF
}

load_coverage_object_args() {
  local input="$1"
  local coverage_object
  local i

  coverage_objects=()
  while IFS= read -r coverage_object; do
    [ -n "${coverage_object}" ] || continue
    coverage_objects+=("${coverage_object}")
  done < "${input}"
  object_count="$(wc -l < "${input}" | tr -d ' ')"
  if [ "${object_count}" = "0" ]; then
    echo "error: no coverage objects found in ${input}" >&2
    exit 1
  fi
  first_object="${coverage_objects[0]}"
  object_args=()
  for ((i = 1; i < ${#coverage_objects[@]}; ++i)); do
    object_args+=("-object=${coverage_objects[$i]}")
  done
}

run_llvm_cov_outputs() {
  local profile="$1"
  local objects="$2"
  local report="$3"
  local json="$4"
  local html_dir="$5"
  local html_enabled="$6"
  local line_counts="$7"
  local label="$8"

  load_coverage_object_args "${objects}"

  printf '\n== llvm-cov report%s ==\n' "${label}"
  "${llvm_cov}" report "${first_object}" "${object_args[@]}" "${ignore_args[@]}" \
    -instr-profile="${profile}" \
    "${repo_root}/dev" | tee "${report}"

  printf '\n== llvm-cov export%s ==\n' "${label}"
  "${llvm_cov}" export "${first_object}" "${object_args[@]}" "${ignore_args[@]}" \
    -instr-profile="${profile}" \
    "${repo_root}/dev" > "${json}"
  printf 'wrote %s\n' "${json}"

  if [ -n "${line_counts}" ]; then
    printf '\n== llvm-cov line counts%s ==\n' "${label}"
    "${llvm_cov}" show "${first_object}" "${object_args[@]}" "${ignore_args[@]}" \
      -instr-profile="${profile}" \
      -format=text \
      -show-line-counts-or-regions \
      "${repo_root}/dev" > "${line_counts}"
    printf 'wrote %s\n' "${line_counts}"
  fi

  if [ "${html_enabled}" -eq 1 ]; then
    printf '\n== llvm-cov html%s ==\n' "${label}"
    rm -rf "${html_dir}"
    "${llvm_cov}" show "${first_object}" "${object_args[@]}" "${ignore_args[@]}" \
      -instr-profile="${profile}" \
      -format=html \
      -output-dir="${html_dir}" \
      "${repo_root}/dev"
    printf 'wrote %s\n' "${html_dir}/index.html"
  fi
}

timestamp="$(date +%Y%m%d-%H%M%S)"
cov_root="${COV_ROOT:-/tmp/cppgm-source-coverage-${timestamp}}"
active_pas="${ACTIVE_TEST_REPORT_PAS:-}"
jobs="${COVERAGE_JOBS:-4}"
subtest_jobs="${TEST_REPORT_SUBTEST_JOBS:-2}"
assignment_jobs="${TEST_REPORT_ASSIGNMENT_JOBS:-}"
strict_pas="${COVERAGE_STRICT_PAS:-pa19 pa20 pa22 pa23 pa24}"
by_pa=0
strict_enabled=1
template_kernel_enabled=1
memory_census_enabled=1
linux_target_enabled=1
host_runtime_enabled=1
host_eh_object_enabled=1
semantic_hotspot_enabled=1
diagnostic_instrumentation_enabled=1
cli_batch_enabled=1
tool_help_enabled=1
skip_build=0
skip_tests=0
write_html=1
cxx="${CXX:-}"
host_cxx="${CPPGM_HOST_CXX:-}"

if [ "${COVERAGE_BY_PA:-}" = "1" ]; then
  by_pa=1
fi
if [ "${COVERAGE_STRICT:-1}" = "0" ]; then
  strict_enabled=0
fi
if [ "${COVERAGE_TEMPLATE_KERNEL:-1}" = "0" ]; then
  template_kernel_enabled=0
fi
if [ "${COVERAGE_MEMORY_CENSUS:-1}" = "0" ]; then
  memory_census_enabled=0
fi
if [ "${COVERAGE_LINUX_TARGET:-1}" = "0" ]; then
  linux_target_enabled=0
fi
if [ "${COVERAGE_HOST_RUNTIME:-1}" = "0" ]; then
  host_runtime_enabled=0
fi
if [ "${COVERAGE_HOST_EH_OBJECT:-1}" = "0" ]; then
  host_eh_object_enabled=0
fi
if [ "${COVERAGE_SEMANTIC_HOTSPOT:-1}" = "0" ]; then
  semantic_hotspot_enabled=0
fi
if [ "${COVERAGE_DIAGNOSTIC_INSTRUMENTATION:-1}" = "0" ]; then
  diagnostic_instrumentation_enabled=0
fi
if [ "${COVERAGE_CLI_BATCH:-1}" = "0" ]; then
  cli_batch_enabled=0
fi
if [ "${COVERAGE_TOOL_HELP:-1}" = "0" ]; then
  tool_help_enabled=0
fi

while [ "$#" -gt 0 ]; do
  case "$1" in
    --root)
      if [ "$#" -lt 2 ]; then echo "missing value for --root" >&2; exit 2; fi
      cov_root="$2"
      shift 2
      ;;
    --pas)
      if [ "$#" -lt 2 ]; then echo "missing value for --pas" >&2; exit 2; fi
      active_pas="$2"
      shift 2
      ;;
    --by-pa)
      by_pa=1
      shift
      ;;
    --jobs)
      if [ "$#" -lt 2 ]; then echo "missing value for --jobs" >&2; exit 2; fi
      jobs="$2"
      shift 2
      ;;
    --subtest-jobs)
      if [ "$#" -lt 2 ]; then echo "missing value for --subtest-jobs" >&2; exit 2; fi
      subtest_jobs="$2"
      shift 2
      ;;
    --assignment-jobs)
      if [ "$#" -lt 2 ]; then echo "missing value for --assignment-jobs" >&2; exit 2; fi
      assignment_jobs="$2"
      shift 2
      ;;
    --strict-pas)
      if [ "$#" -lt 2 ]; then echo "missing value for --strict-pas" >&2; exit 2; fi
      strict_pas="$2"
      shift 2
      ;;
    --skip-strict)
      strict_enabled=0
      shift
      ;;
    --skip-template-kernel)
      template_kernel_enabled=0
      shift
      ;;
    --skip-memory-census)
      memory_census_enabled=0
      shift
      ;;
    --skip-linux-target)
      linux_target_enabled=0
      shift
      ;;
    --skip-host-runtime)
      host_runtime_enabled=0
      shift
      ;;
    --skip-host-eh-object)
      host_eh_object_enabled=0
      shift
      ;;
    --skip-semantic-hotspot)
      semantic_hotspot_enabled=0
      shift
      ;;
    --skip-diagnostic-instrumentation)
      diagnostic_instrumentation_enabled=0
      shift
      ;;
    --skip-cli-batch)
      cli_batch_enabled=0
      shift
      ;;
    --skip-tool-help)
      tool_help_enabled=0
      shift
      ;;
    --cxx)
      if [ "$#" -lt 2 ]; then echo "missing value for --cxx" >&2; exit 2; fi
      cxx="$2"
      shift 2
      ;;
    --host-cxx)
      if [ "$#" -lt 2 ]; then echo "missing value for --host-cxx" >&2; exit 2; fi
      host_cxx="$2"
      shift 2
      ;;
    --skip-build)
      skip_build=1
      shift
      ;;
    --skip-tests)
      skip_tests=1
      shift
      ;;
    --no-html)
      write_html=0
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

llvm_root="$(find_llvm_root)" || {
  echo "error: could not find Homebrew LLVM under /usr/local/opt/llvm or /opt/homebrew/opt/llvm" >&2
  exit 1
}
llvm_cov="${llvm_root}/bin/llvm-cov"
llvm_profdata="${llvm_root}/bin/llvm-profdata"
if [ -z "${cxx}" ]; then
  cxx="${llvm_root}/bin/clang++"
fi
if [ -z "${host_cxx}" ]; then
  host_cxx="${cxx}"
fi

cov_root="$(mkdir -p "${cov_root}" && cd "${cov_root}" && pwd)"
obj_root="${cov_root}/obj"
generated_root="${obj_root}/generated"
profraw_dir="${cov_root}/profraw"
profraw_list="${cov_root}/profraw.list"
objects_list="${cov_root}/coverage-objects.list"
profdata="${cov_root}/all.profdata"
report_txt="${cov_root}/report.txt"
coverage_json="${cov_root}/coverage.json"
line_counts_txt="${cov_root}/line-counts.txt"
summary_txt="${cov_root}/summary.txt"
run_stamp="${cov_root}/run-start.stamp"
by_pa_root="${cov_root}/by-pa"
extra_tools_dir="${cov_root}/extra-tools"
extra_coverage_objects="${cov_root}/extra-coverage-objects.list"
attribution_csv="${cov_root}/line-attribution.csv"
attribution_json="${cov_root}/line-attribution.json"
attribution_summary="${cov_root}/line-attribution-summary.txt"
unhit_lines="${cov_root}/unhit-lines.txt"
review_queue="${cov_root}/review-queue.md"
ignore_regex="${COVERAGE_IGNORE_REGEX:-.*/test_runner\\.cpp$}"
mkdir -p "${obj_root}" "${generated_root}" "${profraw_dir}"
: > "${run_stamp}"
: > "${extra_coverage_objects}"

dev_cc_flags='-std=gnu++11 -Wall -O0 -g -fno-omit-frame-pointer -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)'
pa_cc_flags='-std=gnu++11 -Wall -O0 -g -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS)'

make_common=(
  "OBJ=${obj_root}"
  "GENERATED=${generated_root}"
  "CXX=${cxx}"
  "CPPGM_HOST_CXX=${host_cxx}"
)

tool_names=(
  pptoken
  posttoken
  ctrlexpr
  macro
  preproc
  recog
  lowir2cy86
  lowiropt
  lowir2native
  tmplsolve
  cppgm++
  mobjroundtrip
  nsdecl
  nsinit
  cy86
)

printf 'coverage root: %s\n' "${cov_root}"
printf 'compiler: %s\n' "${cxx}"
printf 'host compiler: %s\n' "${host_cxx}"
printf 'llvm-cov: %s\n' "${llvm_cov}"

if [ "${skip_build}" -eq 0 ]; then
  printf '\n== build coverage binaries ==\n'
  (
    cd "${repo_root}/dev" &&
    env MAKEFLAGS="-j${jobs}" make -s all \
      "${make_common[@]}" \
      "CC_FLAGS=${dev_cc_flags}"
  )
else
  printf '\n== build coverage binaries ==\n'
  printf 'skipped\n'
fi

test_status=0
overall_test_status=0
strict_status=0
template_kernel_status=0
memory_census_status=0
linux_target_status=0
host_runtime_status=0
host_eh_object_status=0
semantic_hotspot_status=0
diagnostic_instrumentation_status=0
cli_batch_status=0
tool_help_status=0
if [ "${skip_tests}" -eq 0 ]; then
  printf '\n== run test-report-nobuild under coverage ==\n'
  rm -f "${profraw_dir}"/*.profraw
  remove_selected_pa_tool_binaries
  test_args=(
    test-report-nobuild
    "${make_common[@]}"
    "CC_FLAGS=${pa_cc_flags}"
    "TEST_REPORT_SUBTEST_JOBS=${subtest_jobs}"
  )
  if [ -n "${assignment_jobs}" ]; then
    test_args+=("TEST_REPORT_ASSIGNMENT_JOBS=${assignment_jobs}")
  fi
  if [ -n "${active_pas}" ]; then
    test_args+=("ACTIVE_TEST_REPORT_PAS=${active_pas}")
  fi
  (
    cd "${repo_root}" &&
    env MAKEFLAGS="-j${jobs}" LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
      make -s "${test_args[@]}"
  )
  test_status=$?
  overall_test_status="${test_status}"
  printf '%s\n' "${test_status}" > "${cov_root}/test-report.status"
  if [ "${test_status}" -ne 0 ]; then
    printf 'warning: test-report-nobuild exited with status %s; continuing with collected profiles\n' "${test_status}" >&2
  fi

  if [ "${strict_enabled}" -eq 1 ]; then
    printf '\n== run strict semantic/witness tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env MAKEFLAGS="-j${jobs}" \
        LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        make -s test-strict-nobuild \
          "${make_common[@]}" \
          "CC_FLAGS=${pa_cc_flags}" \
          "STRICT_PAS=${strict_pas}" \
          "STRICT_SUBTEST_JOBS=${subtest_jobs}"
    )
    strict_status=$?
    printf '%s\n' "${strict_status}" > "${cov_root}/strict.status"
    if [ "${strict_status}" -ne 0 ]; then
      printf 'warning: strict semantic/witness tests exited with status %s; continuing with collected profiles\n' "${strict_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${strict_status}"
      fi
    fi
  else
    printf '%s\n' "${strict_status}" > "${cov_root}/strict.status"
  fi

  if [ "${template_kernel_enabled}" -eq 1 ] && [ -d "${repo_root}/template-kernel" ]; then
    printf '\n== run template-kernel tests under coverage ==\n'
    (
      cd "${repo_root}/template-kernel" &&
      env MAKEFLAGS="-j${jobs}" LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        make -s test \
          TARGET=../dev/tmplsolve \
          "${make_common[@]}" \
          "CC_FLAGS=${pa_cc_flags}"
    )
    template_kernel_status=$?
    printf '%s\n' "${template_kernel_status}" > "${cov_root}/template-kernel.status"
    if [ "${template_kernel_status}" -ne 0 ]; then
      printf 'warning: template-kernel tests exited with status %s; continuing with collected profiles\n' "${template_kernel_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${template_kernel_status}"
      fi
    fi
  else
    printf '%s\n' "${template_kernel_status}" > "${cov_root}/template-kernel.status"
  fi

  if [ "${memory_census_enabled}" -eq 1 ]; then
    printf '\n== run memory census smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_memory_census_smoke.sh "${repo_root}/dev"
    )
    memory_census_status=$?
    printf '%s\n' "${memory_census_status}" > "${cov_root}/memory-census.status"
    if [ "${memory_census_status}" -ne 0 ]; then
      printf 'warning: memory census smoke tests exited with status %s; continuing with collected profiles\n' "${memory_census_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${memory_census_status}"
      fi
    fi
  else
    printf '%s\n' "${memory_census_status}" > "${cov_root}/memory-census.status"
  fi

  if [ "${linux_target_enabled}" -eq 1 ]; then
    printf '\n== run linux target smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_linux_target_smoke.sh "${repo_root}/dev"
    )
    linux_target_status=$?
    printf '%s\n' "${linux_target_status}" > "${cov_root}/linux-target.status"
    if [ "${linux_target_status}" -ne 0 ]; then
      printf 'warning: linux target smoke tests exited with status %s; continuing with collected profiles\n' "${linux_target_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${linux_target_status}"
      fi
    fi
  else
    printf '%s\n' "${linux_target_status}" > "${cov_root}/linux-target.status"
  fi

  if [ "${host_runtime_enabled}" -eq 1 ]; then
    printf '\n== run host builtin runtime smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        CXX="${cxx}" \
        CPPGM_HOST_CXX="${host_cxx}" \
        scripts/run_host_builtin_runtime_smoke.sh "${obj_root}"
    )
    host_runtime_status=$?
    printf '%s\n' "${host_runtime_status}" > "${cov_root}/host-runtime.status"
    if [ "${host_runtime_status}" -ne 0 ]; then
      printf 'warning: host builtin runtime smoke tests exited with status %s; continuing with collected profiles\n' "${host_runtime_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${host_runtime_status}"
      fi
    fi
  else
    printf '%s\n' "${host_runtime_status}" > "${cov_root}/host-runtime.status"
  fi

  if [ "${host_eh_object_enabled}" -eq 1 ]; then
    printf '\n== run host EH object smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_host_eh_object_smoke.sh "${repo_root}/dev"
    )
    host_eh_object_status=$?
    printf '%s\n' "${host_eh_object_status}" > "${cov_root}/host-eh-object.status"
    if [ "${host_eh_object_status}" -ne 0 ]; then
      printf 'warning: host EH object smoke tests exited with status %s; continuing with collected profiles\n' "${host_eh_object_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${host_eh_object_status}"
      fi
    fi
  else
    printf '%s\n' "${host_eh_object_status}" > "${cov_root}/host-eh-object.status"
  fi

  if [ "${semantic_hotspot_enabled}" -eq 1 ]; then
    printf '\n== run semantic hotspot smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_semantic_hotspot_smoke.sh "${repo_root}/dev"
    )
    semantic_hotspot_status=$?
    printf '%s\n' "${semantic_hotspot_status}" > "${cov_root}/semantic-hotspot.status"
    if [ "${semantic_hotspot_status}" -ne 0 ]; then
      printf 'warning: semantic hotspot smoke tests exited with status %s; continuing with collected profiles\n' "${semantic_hotspot_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${semantic_hotspot_status}"
      fi
    fi
  else
    printf '%s\n' "${semantic_hotspot_status}" > "${cov_root}/semantic-hotspot.status"
  fi

  if [ "${diagnostic_instrumentation_enabled}" -eq 1 ]; then
    printf '\n== run diagnostic instrumentation smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_diagnostic_instrumentation_smoke.sh "${repo_root}/dev"
    )
    diagnostic_instrumentation_status=$?
    printf '%s\n' "${diagnostic_instrumentation_status}" > "${cov_root}/diagnostic-instrumentation.status"
    if [ "${diagnostic_instrumentation_status}" -ne 0 ]; then
      printf 'warning: diagnostic instrumentation smoke tests exited with status %s; continuing with collected profiles\n' "${diagnostic_instrumentation_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${diagnostic_instrumentation_status}"
      fi
    fi
  else
    printf '%s\n' "${diagnostic_instrumentation_status}" > "${cov_root}/diagnostic-instrumentation.status"
  fi

  if [ "${cli_batch_enabled}" -eq 1 ]; then
    printf '\n== run cli batch frontend smoke tests under coverage ==\n'
    mkdir -p "${extra_tools_dir}"
    (
      cd "${repo_root}/dev" &&
      env MAKEFLAGS="-j${jobs}" make -s lowiropt \
        "${make_common[@]}" \
        "CC_FLAGS=${dev_cc_flags}" \
        CPPGM_TEST_RUNNER=0
    )
    cli_batch_build_status=$?
    if [ "${cli_batch_build_status}" -eq 0 ]; then
      cp "${repo_root}/dev/lowiropt" "${extra_tools_dir}/lowiropt"
      chmod +x "${extra_tools_dir}/lowiropt"
      printf '%s\n' "${extra_tools_dir}/lowiropt" >> "${extra_coverage_objects}"
      (
        cd "${repo_root}/dev" &&
        env MAKEFLAGS="-j${jobs}" make -s lowiropt \
          "${make_common[@]}" \
          "CC_FLAGS=${dev_cc_flags}" \
          CPPGM_TEST_RUNNER=1
      )
      cli_batch_restore_status=$?
      if [ "${cli_batch_restore_status}" -ne 0 ]; then
        cli_batch_status="${cli_batch_restore_status}"
      else
        (
          cd "${repo_root}" &&
          env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
            scripts/run_cli_batch_frontend_smoke.sh "${extra_tools_dir}"
        )
        cli_batch_status=$?
      fi
    else
      cli_batch_status="${cli_batch_build_status}"
    fi
    printf '%s\n' "${cli_batch_status}" > "${cov_root}/cli-batch.status"
    if [ "${cli_batch_status}" -ne 0 ]; then
      printf 'warning: cli batch frontend smoke tests exited with status %s; continuing with collected profiles\n' "${cli_batch_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${cli_batch_status}"
      fi
    fi
  else
    printf '%s\n' "${cli_batch_status}" > "${cov_root}/cli-batch.status"
  fi

  if [ "${tool_help_enabled}" -eq 1 ]; then
    printf '\n== run tool help smoke tests under coverage ==\n'
    (
      cd "${repo_root}" &&
      env LLVM_PROFILE_FILE="${profraw_dir}/%m-%p.profraw" \
        scripts/run_tool_help_smoke.sh "${repo_root}/dev"
    )
    tool_help_status=$?
    printf '%s\n' "${tool_help_status}" > "${cov_root}/tool-help.status"
    if [ "${tool_help_status}" -ne 0 ]; then
      printf 'warning: tool help smoke tests exited with status %s; continuing with collected profiles\n' "${tool_help_status}" >&2
      if [ "${overall_test_status}" -eq 0 ]; then
        overall_test_status="${tool_help_status}"
      fi
    fi
  else
    printf '%s\n' "${tool_help_status}" > "${cov_root}/tool-help.status"
  fi
else
  printf '\n== run test-report-nobuild under coverage ==\n'
  printf 'skipped\n'
  if [ -f "${cov_root}/test-report.status" ]; then
    test_status="$(cat "${cov_root}/test-report.status")"
  fi
  if [ -f "${cov_root}/strict.status" ]; then
    strict_status="$(cat "${cov_root}/strict.status")"
  fi
  if [ -f "${cov_root}/template-kernel.status" ]; then
    template_kernel_status="$(cat "${cov_root}/template-kernel.status")"
  fi
  if [ -f "${cov_root}/memory-census.status" ]; then
    memory_census_status="$(cat "${cov_root}/memory-census.status")"
  fi
  if [ -f "${cov_root}/linux-target.status" ]; then
    linux_target_status="$(cat "${cov_root}/linux-target.status")"
  fi
  if [ -f "${cov_root}/host-runtime.status" ]; then
    host_runtime_status="$(cat "${cov_root}/host-runtime.status")"
  fi
  if [ -f "${cov_root}/host-eh-object.status" ]; then
    host_eh_object_status="$(cat "${cov_root}/host-eh-object.status")"
  fi
  if [ -f "${cov_root}/semantic-hotspot.status" ]; then
    semantic_hotspot_status="$(cat "${cov_root}/semantic-hotspot.status")"
  fi
  if [ -f "${cov_root}/diagnostic-instrumentation.status" ]; then
    diagnostic_instrumentation_status="$(cat "${cov_root}/diagnostic-instrumentation.status")"
  fi
  if [ -f "${cov_root}/cli-batch.status" ]; then
    cli_batch_status="$(cat "${cov_root}/cli-batch.status")"
  fi
  if [ -f "${cov_root}/tool-help.status" ]; then
    tool_help_status="$(cat "${cov_root}/tool-help.status")"
  fi
  overall_test_status="${test_status}"
  if [ "${overall_test_status}" -eq 0 ] && [ "${strict_status}" -ne 0 ]; then
    overall_test_status="${strict_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${template_kernel_status}" -ne 0 ]; then
    overall_test_status="${template_kernel_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${memory_census_status}" -ne 0 ]; then
    overall_test_status="${memory_census_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${linux_target_status}" -ne 0 ]; then
    overall_test_status="${linux_target_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${host_runtime_status}" -ne 0 ]; then
    overall_test_status="${host_runtime_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${host_eh_object_status}" -ne 0 ]; then
    overall_test_status="${host_eh_object_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${semantic_hotspot_status}" -ne 0 ]; then
    overall_test_status="${semantic_hotspot_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${diagnostic_instrumentation_status}" -ne 0 ]; then
    overall_test_status="${diagnostic_instrumentation_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${cli_batch_status}" -ne 0 ]; then
    overall_test_status="${cli_batch_status}"
  fi
  if [ "${overall_test_status}" -eq 0 ] && [ "${tool_help_status}" -ne 0 ]; then
    overall_test_status="${tool_help_status}"
  fi
fi

find "${profraw_dir}" -type f -name '*.profraw' | sort > "${profraw_list}"
profraw_count="$(wc -l < "${profraw_list}" | tr -d ' ')"
if [ "${profraw_count}" = "0" ]; then
  echo "error: no profraw files found in ${profraw_dir}" >&2
  exit 1
fi

printf '\n== merge profiles ==\n'
"${llvm_profdata}" merge -sparse @"${profraw_list}" -o "${profdata}"

ignore_args=()
if [ -n "${ignore_regex}" ]; then
  ignore_args+=("-ignore-filename-regex=${ignore_regex}")
fi

object_stamp="${run_stamp}"
if [ "${skip_tests}" -eq 1 ]; then
  object_stamp=""
fi
write_coverage_objects_list "${objects_list}" "${object_stamp}"
line_counts_arg=""
if [ "${by_pa}" -eq 1 ]; then
  line_counts_arg="${line_counts_txt}"
fi
run_llvm_cov_outputs "${profdata}" "${objects_list}" "${report_txt}" "${coverage_json}" "${cov_root}/html" "${write_html}" "${line_counts_arg}" ""
main_object_count="${object_count}"

if [ "${by_pa}" -eq 1 ]; then
  printf '\n== per-assignment coverage ==\n'
  mkdir -p "${by_pa_root}"
  selected_pas > "${by_pa_root}/pas.list"
  if [ "${skip_tests}" -eq 1 ]; then
    printf 'skipped per-assignment test runs; reusing existing %s coverage JSON files\n' "${by_pa_root}"
  else
    while IFS= read -r pa; do
      [ -n "${pa}" ] || continue
      pa_dir="${repo_root}/${pa}"
      if [ ! -d "${pa_dir}" ]; then
        printf 'warning: skipping missing assignment directory %s\n' "${pa}" >&2
        continue
      fi

      pa_root="${by_pa_root}/${pa}"
      pa_profraw_dir="${pa_root}/profraw"
      pa_profraw_list="${pa_root}/profraw.list"
      pa_profdata="${pa_root}/all.profdata"
      pa_objects="${pa_root}/coverage-objects.list"
      pa_report="${pa_root}/report.txt"
      pa_json="${pa_root}/coverage.json"
      pa_line_counts="${pa_root}/line-counts.txt"
      pa_summary="${pa_root}/summary.txt"
      pa_stamp="${pa_root}/run-start.stamp"
      mkdir -p "${pa_profraw_dir}"
      rm -f "${pa_profraw_dir}"/*.profraw
      : > "${pa_stamp}"
      remove_one_pa_tool_binaries "${pa}"

      printf '\n-- %s --\n' "${pa}"
      (
        cd "${pa_dir}" &&
        env MAKEFLAGS="-j${jobs}" LLVM_PROFILE_FILE="${pa_profraw_dir}/%m-%p.profraw" \
          make -s test \
            "${make_common[@]}" \
            "CC_FLAGS=${pa_cc_flags}" \
            "CPPGM_TEST_JOBS=${subtest_jobs}" \
            CPPGM_SKIP_DEV_REBUILD=1
      )
      pa_status=$?
      printf '%s\n' "${pa_status}" > "${pa_root}/test.status"
      if [ "${pa_status}" -ne 0 ]; then
        printf 'warning: %s test exited with status %s; continuing with collected profiles\n' "${pa}" "${pa_status}" >&2
      fi

      find "${pa_profraw_dir}" -type f -name '*.profraw' | sort > "${pa_profraw_list}"
      pa_profraw_count="$(wc -l < "${pa_profraw_list}" | tr -d ' ')"
      if [ "${pa_profraw_count}" = "0" ]; then
        printf 'warning: no profraw files found for %s\n' "${pa}" >&2
        continue
      fi

      "${llvm_profdata}" merge -sparse @"${pa_profraw_list}" -o "${pa_profdata}"
      write_pa_coverage_objects_list "${pa_objects}" "${pa}"
      run_llvm_cov_outputs "${pa_profdata}" "${pa_objects}" "${pa_report}" "${pa_json}" "${pa_root}/html" 0 "${pa_line_counts}" " for ${pa}"

      {
        printf 'pa=%s\n' "${pa}"
        printf 'profraw_count=%s\n' "${pa_profraw_count}"
        printf 'object_count=%s\n' "${object_count}"
        printf 'test_status=%s\n' "${pa_status}"
        printf 'report=%s\n' "${pa_report}"
        printf 'json=%s\n' "${pa_json}"
        printf 'line_counts=%s\n' "${pa_line_counts}"
      } > "${pa_summary}"
    done < "${by_pa_root}/pas.list"
  fi

  printf '\n== line attribution ==\n'
  python3 "${repo_root}/scripts/source_coverage_attribution.py" \
    --repo-root "${repo_root}" \
    --full-json "${coverage_json}" \
    --full-text "${line_counts_txt}" \
    --by-pa-dir "${by_pa_root}" \
    --out-csv "${attribution_csv}" \
    --out-json "${attribution_json}" \
    --out-summary "${attribution_summary}" \
    --out-unhit "${unhit_lines}" \
    --out-review "${review_queue}"
  printf 'wrote %s\n' "${attribution_summary}"
fi

{
  printf 'coverage_root=%s\n' "${cov_root}"
  printf 'profraw_count=%s\n' "${profraw_count}"
  printf 'object_count=%s\n' "${main_object_count}"
  printf 'test_report_status=%s\n' "${test_status}"
  printf 'strict_status=%s\n' "${strict_status}"
  printf 'template_kernel_status=%s\n' "${template_kernel_status}"
  printf 'memory_census_status=%s\n' "${memory_census_status}"
  printf 'linux_target_status=%s\n' "${linux_target_status}"
  printf 'host_runtime_status=%s\n' "${host_runtime_status}"
  printf 'host_eh_object_status=%s\n' "${host_eh_object_status}"
  printf 'semantic_hotspot_status=%s\n' "${semantic_hotspot_status}"
  printf 'diagnostic_instrumentation_status=%s\n' "${diagnostic_instrumentation_status}"
  printf 'cli_batch_status=%s\n' "${cli_batch_status}"
  printf 'tool_help_status=%s\n' "${tool_help_status}"
  printf 'overall_test_status=%s\n' "${overall_test_status}"
  printf 'report=%s\n' "${report_txt}"
  printf 'json=%s\n' "${coverage_json}"
  if [ "${by_pa}" -eq 1 ]; then
    printf 'line_counts=%s\n' "${line_counts_txt}"
  fi
  if [ "${write_html}" -eq 1 ]; then
    printf 'html=%s\n' "${cov_root}/html/index.html"
  fi
  if [ "${by_pa}" -eq 1 ]; then
    printf 'by_pa=%s\n' "${by_pa_root}"
    printf 'line_attribution_csv=%s\n' "${attribution_csv}"
    printf 'line_attribution_json=%s\n' "${attribution_json}"
    printf 'line_attribution_summary=%s\n' "${attribution_summary}"
    printf 'unhit_lines=%s\n' "${unhit_lines}"
    printf 'review_queue=%s\n' "${review_queue}"
  fi
} > "${summary_txt}"

printf '\ncoverage summary: %s\n' "${summary_txt}"
if [ "${overall_test_status}" -ne 0 ]; then
  printf 'note: tests had failures; coverage artifacts were still generated\n'
fi

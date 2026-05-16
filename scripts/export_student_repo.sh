#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat >&2 <<'EOF'
usage: scripts/export_student_repo.sh [--force] DEST

Generate a student-facing cppgm repository at DEST.

Options:
  --force   Remove DEST before exporting.

Environment:
  CXX                  Compiler used to build exported reference binaries.
  CPPGM_HOST_CXX       Host compiler recorded in cppgm++ defaults.
  CPPGM_STDLIB_FLAGS   Extra standard-library flags for the host compiler.
  CPPGM_EXPORT_ALLOW_NON_LINUX
                       Set to 1 for local smoke exports on non-Linux hosts.
EOF
}

force=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --force)
      force=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      echo "export_student_repo: unknown option: $1" >&2
      usage
      exit 2
      ;;
    *)
      break
      ;;
  esac
done

if [ "$#" -ne 1 ]; then
  usage
  exit 2
fi

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
repo_root=$(cd "$script_dir/.." && pwd)
dest=$1
if [[ "$dest" != /* ]]; then
  dest=$(cd "$(dirname "$dest")" && pwd)/$(basename "$dest")
fi

host_uname=$(uname -s 2>/dev/null || echo unknown)
if [ "$host_uname" != "Linux" ] && [ "${CPPGM_EXPORT_ALLOW_NON_LINUX:-0}" != "1" ]; then
  echo "export_student_repo: student reference binaries must be generated on Linux" >&2
  echo "export_student_repo: set CPPGM_EXPORT_ALLOW_NON_LINUX=1 only for local smoke exports" >&2
  exit 2
fi

if [ -e "$dest" ]; then
  if [ "$force" -ne 1 ]; then
    echo "export_student_repo: destination exists: $dest" >&2
    echo "export_student_repo: pass --force to replace it" >&2
    exit 2
  fi
  rm -rf "$dest"
fi

mkdir -p "$dest"

copy_tracked_paths() {
  (
    cd "$repo_root"
    git ls-files -z -- "$@" |
      perl -0ne '
        chomp;
        next if m{(^|/)[^/]+-ref$};
        next if m{\.py$};
        next if m{\.diff$};
        next if m{(^|/)[^/]+\.my(?:\.|$)};
        next if m{^pa9/extras/};
        next if m{^pa32/tests/general/200-host-compact-unwind-large-frame-fallback(?:\.|$)};
        print "$_\0";
      '
  ) | rsync -a --from0 --files-from=- "$repo_root/" "$dest/"
}

sanitize_student_makefile_defaults() {
  perl -0pi -e '
    s/GNUMAKE = \$\(firstword \$\(wildcard \/opt\/homebrew\/opt\/make\/libexec\/gnubin\/make \/usr\/local\/opt\/make\/libexec\/gnubin\/make\)\)\nifneq \(\$\(GNUMAKE\),\)\nMAKE := \$\(GNUMAKE\)\nendif\n//g;
    s/sysctl -n hw\.ncpu 2>\/dev\/null \|\| //g;
    s/LLVM_CXX_LOCAL = \/usr\/local\/opt\/llvm\/bin\/clang\+\+\nLLVM_CXX_HOMEBREW = \/opt\/homebrew\/opt\/llvm\/bin\/clang\+\+\n(?:ifeq \(\$\(HOST_UNAME_S\),Darwin\)\n)?ifeq \(\$\(wildcard \$\(LLVM_CXX_LOCAL\)\),\)\nifneq \(\$\(wildcard \$\(LLVM_CXX_HOMEBREW\)\),\)\nLLVM_CXX_DEFAULT = \$\(LLVM_CXX_HOMEBREW\)\nendif\nelse\nLLVM_CXX_DEFAULT = \$\(LLVM_CXX_LOCAL\)\nendif\n(?:ifdef LLVM_CXX_DEFAULT\nHOST_CXX_DEFAULT = \$\(LLVM_CXX_DEFAULT\)\nelse\nHOST_CXX_DEFAULT = clang\+\+\nendif\nelse\nHOST_CXX_DEFAULT = g\+\+\nendif\n|)//g;
    s/ifeq \(\$\(origin CXX\), default\)\n(?:ifdef LLVM_CXX_DEFAULT\nCXX := \$\(LLVM_CXX_DEFAULT\)\nelse\nCXX := (?:clang|g)\+\+\nendif|CXX := \$\(HOST_CXX_DEFAULT\))\nendif/ifeq (\$(origin CXX), default)\nCXX := g++\nendif/g;
    s/ifeq \(\$\(origin CPPGM_HOST_CXX\), undefined\)\nifeq \(\$\(abspath \$\(CXX\)\),\$\(abspath \.\.\/dev\/cppgm\+\+\)\)\n(?:ifdef LLVM_CXX_DEFAULT\n\tCPPGM_HOST_CXX := \$\(LLVM_CXX_DEFAULT\)\nelse\n\tCPPGM_HOST_CXX := clang\+\+\nendif|CPPGM_HOST_CXX := \$\(HOST_CXX_DEFAULT\))\nelse\n\t?CPPGM_HOST_CXX := \$\(CXX\)\nendif\nendif/ifeq (\$(origin CPPGM_HOST_CXX), undefined)\nifeq (\$(abspath \$(CXX)),\$(abspath ..\/dev\/cppgm++))\n\tCPPGM_HOST_CXX := g++\nelse\n\tCPPGM_HOST_CXX := \$(CXX)\nendif\nendif/g;
    s/PA37_LINKER_DETERMINISM_FLAGS =\nifeq \(\$\(HOST_UNAME_S\),Darwin\)\nPA37_LINKER_DETERMINISM_FLAGS \+= -Wl,-reproducible\nendif/PA37_LINKER_DETERMINISM_FLAGS =/g;
  ' "$@"
}

sanitize_linux_student_scripts() {
  local run_script
  run_script="$dest/pa30/scripts/run_one_test.sh"
  if [ -f "$run_script" ]; then
    perl -0pi -e '
      s/host_target_name\(\) \{\n\tcase "\$\(uname -s\)" in\n\t\tDarwin\) printf '"'"'%s\\n'"'"' macos ;;\n\t\tLinux\) printf '"'"'%s\\n'"'"' linux ;;\n\t\t\*\) printf '"'"'%s\\n'"'"' "" ;;\n\tesac\n\}/host_target_name() {\n\tprintf '"'"'%s\\n'"'"' linux\n}/g;
      s/host_target_triple_name\(\) \{\n\tcase "\$\(uname -s\)" in\n\t\tDarwin\) printf '"'"'%s\\n'"'"' x86_64-apple-darwin ;;\n\t\tLinux\) printf '"'"'%s\\n'"'"' x86_64-unknown-linux-gnu ;;\n\t\t\*\) printf '"'"'%s\\n'"'"' "" ;;\n\tesac\n\}/host_target_triple_name() {\n\tprintf '"'"'%s\\n'"'"' x86_64-unknown-linux-gnu\n}/g;
    ' "$run_script"
  fi

  for run_script in "$dest"/pa31/scripts/run_one_test.sh "$dest"/pa32/scripts/run_one_test.sh; do
    [ -f "$run_script" ] || continue
    perl -0pi -e '
      s/\nif ! command -v rg >\/dev\/null 2>&1; then\n\texport PATH="\$repo_root\/scripts\/test_shell_tools:\$PATH"\nfi\n/\n/g;
      s/host_os=\$\(uname -s\)\nshared_ext=so\nhost_tag=\$\(printf '"'"'%s'"'"' "\$host_os" \| tr '"'"'\[:upper:\]'"'"' '"'"'\[:lower:\]'"'"'\)\nif \[ "\$host_os" = "Darwin" \]; then\n\tshared_ext=dylib\n\thost_tag=macos\nfi/host_os=Linux\nshared_ext=so\nhost_tag=linux/g;
      s/host_cxx="\$\{CXX:-c\+\+\}"/host_cxx="\${CXX:-g++}"/g;
      s/\*\.__SHARED_EXT__\|\*\.dylib\|\*\.so/*.__SHARED_EXT__|*.so/g;
      s/\*\.o\|\*\.a\|\*\.dylib\|\*\.so/*.o|*.a|*.so/g;
      s/\|\\\.dylib//g;
    ' "$run_script"
    perl -pi -e '
      if ($skip_darwin_shared) {
        if (/^\t\t\telse$/) {
          $skip_darwin_shared = 0;
          $skip_next_nested_fi = 1;
        }
        $_ = "";
      } elsif (/^\t\t\tif \[ "\$host_os" = "Darwin" \]; then$/) {
        $skip_darwin_shared = 1;
        $_ = "";
      } elsif ($skip_next_nested_fi && /^\t\t\tfi$/) {
        $skip_next_nested_fi = 0;
        $_ = "";
      }
    ' "$run_script"
  done

  run_script="$dest/scripts/run_cpphostinterop_tests_worker.pl"
  if [ -f "$run_script" ]; then
    perl -0pi -e '
      s/\treturn '"'"'macos'"'"' if \$os eq '"'"'darwin'"'"';\n//g;
      s/\tmy \$shared_ext = `uname -s`;\n\tchomp\(\$shared_ext\);\n\t\$shared_ext = \$shared_ext eq '"'"'Darwin'"'"' \? '"'"'dylib'"'"' : '"'"'so'"'"';/\tmy \$shared_ext = '"'"'so'"'"';/g;
      s/\|\\\.dylib//g;
      s/\|dylib//g;
      s@\$flag =~ m/\(\?:\\\.__SHARED_EXT__\|\\\.dylib\|\\\.so\)\$/@\$flag =~ m/(?:\\.__SHARED_EXT__|\\.so)$/@g;
    ' "$run_script"
    perl -pi -e '
      if ($skip_dylib_cmd) {
        if (/^\t\t\t\t: \(\@\{\$ctx->\{host_cc\}\}, '"'"'-shared'"'"', '"'"'-o'"'"', \$sharedlib, \$obj\);$/) {
          $_ = "\t\t\tmy \@cmd = (\@{\$ctx->{host_cc}}, '"'"'-shared'"'"', '"'"'-o'"'"', \$sharedlib, \$obj);\n";
          $skip_dylib_cmd = 0;
        } else {
          $_ = "";
        }
      } elsif (/^\t\t\tmy \@cmd = \$ctx->\{shared_ext\} eq '"'"'dylib'"'"'$/) {
        $skip_dylib_cmd = 1;
        $_ = "";
      }
    ' "$run_script"
  fi

  run_script="$dest/pa13/scripts/cppgm-debugger-common.sh"
  if [ -f "$run_script" ]; then
    perl -0pi -e '
      s/cppgm_sys=\$\(uname -s 2>\/dev\/null \|\| echo unknown\)\n      case "\$cppgm_sys" in\n        Darwin\)\n          cppgm_try_lldb \|\| cppgm_try_gdb \|\| \{\n            echo "missing debugger backend" >&2\n            return 1\n          \}\n          ;;\n        \*\)\n          cppgm_try_gdb \|\| cppgm_try_lldb \|\| \{\n            echo "missing debugger backend" >&2\n            return 1\n          \}\n          ;;\n      esac/cppgm_try_gdb || cppgm_try_lldb || {\n        echo "missing debugger backend" >&2\n        return 1\n      }/g;
    ' "$run_script"
  fi
}

pa_dirs=()
for n in $(seq 1 37); do
  pa_dirs+=("pa$n")
done

shared_scripts=(
  scripts/CppgmBatchWorker.pm
  scripts/check_object_expectations.pl
  scripts/compare_host_defined_symbols.pl
  scripts/compare_results_common.pl
  scripts/compare_witness_results.pl
  scripts/cppgm-cmake-wrapper.sh
  scripts/pa_run_check_targets.mk
  scripts/run_all_tests_common.pl
  scripts/run_cpphostcompat_compile_worker.pl
  scripts/run_cpphostcompat_preproc_worker.pl
  scripts/run_cpphostinterop_tests_worker.pl
  scripts/run_cpptoolchain_tests_worker.pl
  scripts/run_lowir_link_tests_worker.pl
  scripts/run_lowir_native_tests_worker.pl
  scripts/run_witness_tests.pl
  scripts/write_unresolved_symbol_report.pl
)

dev_public=(
  dev/.gitignore
  dev/gen_builtin_host_config.pl
)

dev_support_files=(
  dev/src/DebugPPTokenStream.h
  dev/src/IPPTokenStream.h
  dev/src/exceptions.h
  dev/src/test_runner.cpp
  dev/src/tool_help_text.h
)

copy_tracked_paths \
  .gitignore \
  Makefile \
  doc \
  cppgm.tests \
  "${pa_dirs[@]}" \
  "${shared_scripts[@]}" \
  "${dev_public[@]}" \
  "${dev_support_files[@]}"

for root_doc in "$repo_root"/docs/student-export-root/*.md; do
  install -m 0644 "$root_doc" "$dest/$(basename "$root_doc")"
done
install -m 0644 "$repo_root"/docs/student-export-root/LICENSE "$dest/LICENSE"
install -m 0644 "$repo_root"/docs/student-export-root/AUTHORS "$dest/AUTHORS"
install -m 0644 "$repo_root"/docs/student-export-root/NOTICE "$dest/NOTICE"

scaffold_pairs=(
  pptoken:pptoken-scaffold.cpp
  posttoken:posttoken-scaffold.cpp
  ctrlexpr:ctrlexpr-scaffold.cpp
  macro:macro-scaffold.cpp
  preproc:preproc-scaffold.cpp
  recog:recog-scaffold.cpp
  nsdecl:nsdecl-scaffold.cpp
  nsinit:nsinit-scaffold.cpp
  cy86:cy86-scaffold.cpp
  cppgm++:cppgm++-scaffold.cpp
  lowir2cy86:lowir2cy86-scaffold.cpp
  lowir2native:lowir2native-scaffold.cpp
  cpplink:cpplink-scaffold.cpp
  cppeh:cppeh-scaffold.cpp
  lowiropt:lowiropt-scaffold.cpp
)

for pair in "${scaffold_pairs[@]}"; do
  target=${pair%%:*}
  scaffold=${pair#*:}
  cp -p "$repo_root/dev/$scaffold" "$dest/dev/$target.cpp"
done

cat > "$dest/dev/Makefile" <<'EOF'
DEFAULT_BUILD_JOBS = $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1)
ifeq ($(findstring -j,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(DEFAULT_BUILD_JOBS)
endif

TARGETS = \
	pptoken \
	posttoken \
	ctrlexpr \
	macro \
	preproc \
	recog \
	lowir2cy86 \
	lowiropt \
	lowir2native \
	cppeh \
	cpplink \
	cppgm++ \
	nsdecl \
	nsinit \
	cy86

CXX ?= g++
CPPGM_TEST_RUNNER ?= 1
CPPGM_STDLIB_FLAGS ?=
CC_FLAGS ?= -std=gnu++11 -Wall -O3 $(CPPGM_STDLIB_FLAGS)
TEST_RUNNER_SHARED_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-DTEST_RUNNER_ENABLE,)
TEST_RUNNER_ENTRY_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-Dmain=test_runner_real_main,)
ENTRY_CC_FLAGS = $(CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
SRC = src
OBJ ?= ../obj
COMMON_SRC = $(filter-out $(SRC)/test_runner.cpp,$(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/*/*.cpp))
COMMON_OBJ = $(patsubst $(SRC)/%.cpp,$(OBJ)/student/%.o,$(COMMON_SRC))
RUNNER_OBJ = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),$(OBJ)/student/test_runner_enabled.o)
LINK_OBJ = $(COMMON_OBJ) $(RUNNER_OBJ)
INC = -I$(SRC)

all: $(TARGETS)

$(TARGETS): %: %.cpp $(LINK_OBJ)
	$(CXX) $(ENTRY_CC_FLAGS) $(INC) -o $@ $< $(LINK_OBJ)

$(OBJ)/student/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CC_FLAGS) $(INC) -c -o $@ $<

$(OBJ)/student/test_runner_enabled.o: $(SRC)/test_runner.cpp
	@mkdir -p $(@D)
	$(CXX) $(CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(INC) -c -o $@ $<

clean:
	-rm -f $(TARGETS)
	-rm -rf $(OBJ)/student

.PHONY: all clean
EOF

sanitize_student_makefile_defaults \
  "$dest/Makefile" \
  "$dest"/pa33/Makefile \
  "$dest"/pa34/Makefile \
  "$dest"/pa35/Makefile \
  "$dest"/pa36/Makefile \
  "$dest"/pa37/Makefile
sanitize_linux_student_scripts

for pa_makefile in "$dest"/pa{1..36}/Makefile; do
  [ -f "$pa_makefile" ] || continue
  perl -0pi -e '
    s/^include \.\.\/dev\/frontend_source_sets\.mk\n//mg;
    s/^common_obj_basenames = \$\(FRONTEND_OBJ_BASENAMES_\$\(TARGET\)\)$/common_obj_basenames = \$(patsubst ..\/dev\/src\/%.cpp,%,\$(filter-out ..\/dev\/src\/test_runner.cpp,\$(wildcard ..\/dev\/src\/*.cpp) \$(wildcard ..\/dev\/src\/*\/*.cpp)))/mg;
  ' "$pa_makefile"
done

reference_targets=(
  pptoken
  posttoken
  ctrlexpr
  macro
  preproc
  recog
  nsdecl
  nsinit
  cy86
  cppgm++
  lowir2cy86
  lowir2native
  cpplink
  cppeh
  lowiropt
)

echo "==> Building reference binaries"
make -s -C "$repo_root/dev" all \
  CXX="${CXX:-g++}" \
  CPPGM_HOST_CXX="${CPPGM_HOST_CXX:-${CXX:-g++}}" \
  CPPGM_STDLIB_FLAGS="${CPPGM_STDLIB_FLAGS:-}" \
  OBJ="$repo_root/obj/export-reference-build" \
  OBJECT_ROOT_DEFAULT_DEF=

mkdir -p "$dest/reference-binaries"
for target in "${reference_targets[@]}"; do
  if [ ! -x "$repo_root/dev/$target" ]; then
    echo "export_student_repo: missing built reference binary: dev/$target" >&2
    exit 1
  fi
  install -m 0755 "$repo_root/dev/$target" "$dest/reference-binaries/$target"
  ln -s "../reference-binaries/$target" "$dest/dev/$target-ref"
done

pa_ref_pairs=(
  pa1:pptoken
  pa2:posttoken
  pa3:ctrlexpr
  pa4:macro
  pa5:preproc
  pa6:recog
  pa7:nsdecl
  pa8:nsinit
  pa9:cy86
  pa10:cppgm++
  pa11:cppgm++
  pa12:cppgm++
  pa13:lowir2cy86
  pa14:cppgm++
  pa15:cppgm++
  pa16:cppgm++
  pa17:cppgm++
  pa18:cppgm++
  pa19:cppgm++
  pa20:cppgm++
  pa21:cppgm++
  pa22:cppgm++
  pa23:lowir2native
  pa24:cpplink
  pa25:cppeh
  pa26:cppgm++
  pa27:cppgm++
  pa28:cppgm++
  pa29:cppgm++
  pa30:cppgm++
  pa31:cppgm++
  pa32:cppgm++
  pa33:cppgm++
  pa34:cppgm++
  pa35:lowiropt
  pa36:lowir2native
)

for pair in "${pa_ref_pairs[@]}"; do
  pa=${pair%%:*}
  target=${pair#*:}
  ln -s "../reference-binaries/$target" "$dest/$pa/$target-ref"
done

echo "==> Regenerating reference outputs"
make -s -C "$dest" ref-test \
  CXX="${CXX:-g++}" \
  CPPGM_HOST_CXX="${CPPGM_HOST_CXX:-${CXX:-g++}}" \
  CPPGM_STDLIB_FLAGS="${CPPGM_STDLIB_FLAGS:-}" \
  CPPGM_TEST_RUNNER=1
make -s -C "$dest" ref-test-strict \
  CXX="${CXX:-g++}" \
  CPPGM_HOST_CXX="${CPPGM_HOST_CXX:-${CXX:-g++}}" \
  CPPGM_STDLIB_FLAGS="${CPPGM_STDLIB_FLAGS:-}" \
  CPPGM_TEST_RUNNER=1
make -s -C "$dest" ref-test-debuginfo \
  CXX="${CXX:-g++}" \
  CPPGM_HOST_CXX="${CPPGM_HOST_CXX:-${CXX:-g++}}" \
  CPPGM_STDLIB_FLAGS="${CPPGM_STDLIB_FLAGS:-}" \
  CPPGM_TEST_RUNNER=1

(
  cd "$dest"
  git init -q
  git add -A
  git \
    -c user.name="cppgm export" \
    -c user.email="cppgm-export@example.invalid" \
    commit -q -m "Initial student export"
)

echo "==> Exported student repo to $dest"

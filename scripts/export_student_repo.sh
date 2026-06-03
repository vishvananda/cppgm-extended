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
  CPPGM_REFERENCE_BUNDLE_OUT
                       Path where the reference-binary bundle should be written.
  CPPGM_REFERENCE_BUNDLE_URL
                       URL embedded in the student manifest for auto-download.
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
        next if m{^pa31/tests/general/200-host-compact-unwind-large-frame-fallback(?:\.|$)};
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
    s/PA38_LINKER_DETERMINISM_FLAGS =\nifeq \(\$\(HOST_UNAME_S\),Darwin\)\nPA38_LINKER_DETERMINISM_FLAGS \+= -Wl,-reproducible\nendif/PA38_LINKER_DETERMINISM_FLAGS =/g;
  ' "$@"
}

sanitize_linux_student_scripts() {
  local run_script
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
for n in $(seq 1 38); do
  pa_dirs+=("pa$n")
done

shared_scripts=(
  scripts/CppgmBatchWorker.pm
  scripts/check_object_expectations.pl
  scripts/compare_host_defined_symbols.pl
  scripts/compare_results_common.pl
  scripts/compare_witness_results.pl
  scripts/cppgm-cmake-wrapper.sh
  scripts/dump_host_eh_object_facts.pl
  scripts/ensure_reference_binaries.pl
  scripts/pa_run_check_targets.mk
  scripts/run_all_tests_common.pl
  scripts/run_cpphostcompat_compile_worker.pl
  scripts/run_cpphostcompat_preproc_worker.pl
  scripts/run_cpphostinterop_tests_worker.pl
  scripts/run_cpptoolchain_tests_worker.pl
  scripts/run_lowir_link_tests_worker.pl
  scripts/run_lowir_native_tests_worker.pl
  scripts/run_reference_binary.sh
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
  dev/src/abi_mangle.h
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
  abimangle:abimangle-scaffold.cpp
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
  lowiropt:lowiropt-scaffold.cpp
)

for pair in "${scaffold_pairs[@]}"; do
  target=${pair%%:*}
  scaffold=${pair#*:}
  cp -p "$repo_root/dev/$scaffold" "$dest/dev/$target.cpp"
done

cat > "$dest/dev/frontend_source_sets.mk" <<'EOF'
# Per-tool implementation source lists for the compiler.
#
# Add dev/src/foo.cpp to the tools that use it by adding `foo` below. For
# subdirectories, use the path without `.cpp`, such as `parser/foo`.

FRONTEND_SOURCE_SET_TARGETS := abimangle pptoken posttoken ctrlexpr macro preproc recog nsdecl nsinit cy86 cppgm++ lowiropt lowir2cy86 lowir2native

FRONTEND_OBJ_BASENAMES_abimangle :=
FRONTEND_OBJ_BASENAMES_pptoken :=
FRONTEND_OBJ_BASENAMES_posttoken :=
FRONTEND_OBJ_BASENAMES_ctrlexpr :=
FRONTEND_OBJ_BASENAMES_macro :=
FRONTEND_OBJ_BASENAMES_preproc :=
FRONTEND_OBJ_BASENAMES_recog :=
FRONTEND_OBJ_BASENAMES_nsdecl :=
FRONTEND_OBJ_BASENAMES_nsinit :=
FRONTEND_OBJ_BASENAMES_cy86 :=
FRONTEND_OBJ_BASENAMES_cppgm++ :=
FRONTEND_OBJ_BASENAMES_lowiropt :=
FRONTEND_OBJ_BASENAMES_lowir2cy86 :=
FRONTEND_OBJ_BASENAMES_lowir2native :=
EOF

cat > "$dest/dev/Makefile" <<'EOF'
DEFAULT_BUILD_JOBS = $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1)
ifeq ($(findstring -j,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(DEFAULT_BUILD_JOBS)
endif

TARGETS = \
	abimangle \
	pptoken \
	posttoken \
	ctrlexpr \
	macro \
	preproc \
	recog \
	lowir2cy86 \
	lowiropt \
	lowir2native \
	cppgm++ \
	nsdecl \
	nsinit \
	cy86

CXX ?= g++
V ?= 0
Q = $(if $(filter 1,$(V)),,@)
quiet = @if [ "$(V)" != "1" ]; then printf '  %-7s %s\n' '$(1)' '$(2)'; fi
CPPGM_TEST_RUNNER ?= 1
CPPGM_STDLIB_FLAGS ?=
CC_FLAGS ?= -std=gnu++11 -Wall -O3 $(CPPGM_STDLIB_FLAGS)
TEST_RUNNER_SHARED_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-DTEST_RUNNER_ENABLE,)
TEST_RUNNER_ENTRY_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-Dmain=test_runner_real_main,)
ENTRY_CC_FLAGS = $(CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
SRC = src
OBJ ?= ../obj
OBJDIR = $(OBJ)/dev
DEPDIR = $(OBJDIR)/.d
INC = -I$(SRC)

include frontend_source_sets.mk
$(foreach target,$(TARGETS),$(if $(filter undefined,$(origin FRONTEND_OBJ_BASENAMES_$(target))),$(error missing FRONTEND_OBJ_BASENAMES_$(target) in frontend_source_sets.mk),))

frontend_obj_basenames = $(FRONTEND_OBJ_BASENAMES_$(1))
frontend_objs = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(call frontend_obj_basenames,$(1))))
entry_obj = $(OBJDIR)/entry/$(1).o
runner_obj = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),$(OBJDIR)/test_runner_enabled.o)
link_objs = $(call entry_obj,$(1)) $(call frontend_objs,$(1)) $(call runner_obj)
all_obj_basenames = $(sort $(foreach target,$(TARGETS),$(call frontend_obj_basenames,$(target))))
COMPILE_CONFIG_STAMP = $(OBJDIR)/.compile_config
RUNNER_STATE_STAMP = $(OBJDIR)/.test_runner_mode

all: $(TARGETS)

define FRONTEND_RULES
$(1): $(OBJDIR) $(call link_objs,$(1)) $(RUNNER_STATE_STAMP)
	$(call quiet,LINK,$$@)
	$(Q)$(CXX) $(CC_FLAGS) $(INC) -o $$@ $(call link_objs,$(1))

$(call entry_obj,$(1)): $(1).cpp $(COMPILE_CONFIG_STAMP)
	@mkdir -p $$(@D) $(DEPDIR)/entry
	$(call quiet,CXX,$$@)
	$(Q)$(CXX) $(ENTRY_CC_FLAGS) $(INC) -c -MT $$@ -MMD -MP -MF $(DEPDIR)/entry/$(1).Td -o $$@ $$<
	@mv -f $(DEPDIR)/entry/$(1).Td $(DEPDIR)/entry/$(1).d
endef

$(foreach target,$(TARGETS),$(eval $(call FRONTEND_RULES,$(target))))

$(OBJDIR):
	@mkdir -p $@

FORCE:

$(COMPILE_CONFIG_STAMP): FORCE | $(OBJDIR)
	@printf '%s\n%s\n' "$(CC_FLAGS)" "$(ENTRY_CC_FLAGS)" > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

$(RUNNER_STATE_STAMP): FORCE | $(OBJDIR)
	@current=$$(cat $@ 2>/dev/null || true); \
	if [ "$$current" != "$(CPPGM_TEST_RUNNER)" ]; then \
		printf '%s\n' '$(CPPGM_TEST_RUNNER)' > $@; \
	fi

$(OBJDIR)/test_runner_enabled.o: $(SRC)/test_runner.cpp $(COMPILE_CONFIG_STAMP)
	@mkdir -p $(@D) $(DEPDIR)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(DEPDIR)/test_runner_enabled.Td -o $@ $<
	@mv -f $(DEPDIR)/test_runner_enabled.Td $(DEPDIR)/test_runner_enabled.d

$(OBJDIR)/%.o: $(SRC)/%.cpp $(COMPILE_CONFIG_STAMP)
	@mkdir -p $(@D) $(dir $(DEPDIR)/$*)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(CC_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

clean:
	-rm -f $(TARGETS)
	-rm -rf $(OBJDIR)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(addprefix $(DEPDIR)/,$(addsuffix .d,$(all_obj_basenames)))
-include $(addprefix $(DEPDIR)/entry/,$(addsuffix .d,$(TARGETS)))
-include $(DEPDIR)/test_runner_enabled.d

.PHONY: all clean FORCE
EOF

sanitize_student_makefile_defaults \
  "$dest/Makefile" \
  "$dest"/pa34/Makefile \
  "$dest"/pa36/Makefile \
  "$dest"/pa37/Makefile \
  "$dest"/pa38/Makefile \
  "$dest"/pa39/Makefile
sanitize_linux_student_scripts

perl -0pi -e '
  s/\$\(foreach checkpoint,\$\(CHECKPOINTS\),\$\(if \$\(strip \$\(FRONTEND_OBJ_BASENAMES_\$\(checkpoint\)\)\),,\$\(error missing FRONTEND_OBJ_BASENAMES_\$\(checkpoint\) in \.\.\/dev\/frontend_source_sets\.mk\)\)\)/\$(foreach checkpoint,\$(CHECKPOINTS),\$(if \$(filter undefined,\$(origin FRONTEND_OBJ_BASENAMES_\$(checkpoint))),\$(error missing FRONTEND_OBJ_BASENAMES_\$(checkpoint) in ..\/dev\/frontend_source_sets.mk),))/g;
' "$dest/pa39/Makefile"

cat >> "$dest/Makefile" <<'EOF'

reference-binaries:
	@scripts/ensure_reference_binaries.pl

setup: reference-binaries

.PHONY: reference-binaries setup
EOF

reference_targets=(
  abimangle
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
  pa23:cppgm++
  pa24:cppgm++
  pa25:cppgm++
  pa26:cppgm++
  pa27:cppgm++
  pa28:lowir2native
  pa29:cppgm++
  pa30:abimangle
  pa31:cppgm++
  pa32:cppgm++
  pa33:cppgm++
  pa34:cppgm++
  pa36:cppgm++
  pa37:lowiropt
  pa38:lowir2native
)

source_sha=$(git -C "$repo_root" rev-parse HEAD 2>/dev/null || echo unknown)
source_short_sha=$(git -C "$repo_root" rev-parse --short=12 HEAD 2>/dev/null || echo unknown)
reference_bundle_name=${CPPGM_REFERENCE_BUNDLE_NAME:-cppgm-reference-binaries-linux-x86_64-${source_short_sha}.tar.gz}
reference_bundle_url=${CPPGM_REFERENCE_BUNDLE_URL:-https://github.com/vishvananda/cppgm-extended/releases/download/reference-binaries/${reference_bundle_name}}
reference_bundle_out=${CPPGM_REFERENCE_BUNDLE_OUT:-$repo_root/obj/export-reference-bundles/${reference_bundle_name}}

write_reference_manifest() {
  local bundle_sha target target_sha
  bundle_sha=$(sha256sum "$reference_bundle_out" | awk '{print $1}')
  {
    printf '# cppgm reference binary manifest\n'
    printf 'version\t1\n'
    printf 'source_sha\t%s\n' "$source_sha"
    printf 'platform\tlinux-x86_64\n'
    printf 'bundle_name\t%s\n' "$reference_bundle_name"
    printf 'bundle_url\t%s\n' "$reference_bundle_url"
    printf 'bundle_sha256\t%s\n' "$bundle_sha"
    for target in "${reference_targets[@]}"; do
      target_sha=$(sha256sum "$dest/reference-binaries/$target" | awk '{print $1}')
      printf 'binary\t%s\t%s\n' "$target" "$target_sha"
    done
  } > "$dest/reference-binaries/manifest.tsv"
}

finalize_reference_binaries() {
  local pa target pair

  echo "==> Packaging reference binaries"
  mkdir -p "$(dirname "$reference_bundle_out")"
  (
    cd "$dest/reference-binaries"
    tar -czf "$reference_bundle_out" "${reference_targets[@]}"
  )
  write_reference_manifest
  cat > "$dest/reference-binaries/.gitignore" <<'EOF'
*
!.gitignore
!manifest.tsv
EOF

  for target in "${reference_targets[@]}"; do
    rm -f "$dest/reference-binaries/$target"
    rm -f "$dest/dev/$target-ref"
    ln -s "../scripts/run_reference_binary.sh" "$dest/dev/$target-ref"
  done

  for pair in "${pa_ref_pairs[@]}"; do
    pa=${pair%%:*}
    target=${pair#*:}
    rm -f "$dest/$pa/$target-ref"
    ln -s "../scripts/run_reference_binary.sh" "$dest/$pa/$target-ref"
  done

  echo "==> Wrote reference binary bundle to $reference_bundle_out"
}

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

finalize_reference_binaries

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

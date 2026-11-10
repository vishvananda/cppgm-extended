# Required integrated assembly, before hosted compatibility.
include ../scripts/test_run_id.mk
TARGET = cppgm++
CPPGM_TEST_APP ?= ../dev/$(TARGET)
REF_TEST_APP ?= $(CPPGM_TEST_APP)-ref
LOWIR_APP ?= ../dev/lowir
LOWIR2NATIVE_APP ?= ../dev/lowir2native
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_STDLIB_FLAGS ?=
CPPGM_ASSEMBLY_LEVELS ?= O0
TEST_DEPS ?= dev-shared-target
export CXX CPPGM_HOST_CXX CPPGM_STDLIB_FLAGS CPPGM_ASSEMBLY_LEVELS
export CPPGM_ASSEMBLY_LOWIR_APP := $(LOWIR_APP)
export CPPGM_ASSEMBLY_NATIVE_APP := $(LOWIR2NATIVE_APP)
TEST ?= tests

.PHONY: all test test-batch test-nobuild ref-test ref-test-batch dev-shared-target
all: dev-shared-target

dev-shared-target:
	@if [ "$(CPPGM_SKIP_DEV_REBUILD)" != "1" ]; then $(MAKE) -C ../dev cppgm++ lowir lowir2native CXX="$(CXX)"; fi

test test-batch: $(TEST_DEPS)

test test-batch test-nobuild:
	perl ../scripts/run_all_tests_common.pl control_t "$(CPPGM_TEST_APP)" my "$(TEST)"
	perl ../scripts/compare_results_common.pl control_t ref my "$(TEST)"

# Self-checking controls carry behavioral expectations, not generated payloads.
ref-test ref-test-batch:
	@echo "PA29 controls are self-checking; PA8 regenerates transport references."

CHECK_RUN = perl ../scripts/run_all_tests_common.pl control_t "$(CPPGM_TEST_APP)" check "$(TEST)"
CHECK_COMPARE = perl ../scripts/compare_results_common.pl control_t ref check "$(TEST)"
include ../scripts/pa_run_check_targets.mk

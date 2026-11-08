# Optional assembly extension, outside the required cumulative ladder.
include ../scripts/test_run_id.mk
CPPGM_TEST_APP ?= ../dev/cppgm++
LOWIR_APP ?= ../dev/lowir
LOWIR2NATIVE_APP ?= ../dev/lowir2native
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_STDLIB_FLAGS ?=
CPPGM_ASSEMBLY_LEVELS ?= O0,O3
export CXX CPPGM_HOST_CXX CPPGM_STDLIB_FLAGS CPPGM_ASSEMBLY_LEVELS
export CPPGM_ASSEMBLY_LOWIR_APP := $(LOWIR_APP)
export CPPGM_ASSEMBLY_NATIVE_APP := $(LOWIR2NATIVE_APP)
TEST ?= tests

.PHONY: all test check test-nobuild
all:
	$(MAKE) -C ../dev cppgm++ lowir lowir2native

test check: all
	$(MAKE) test-nobuild

test-nobuild:
	perl ../scripts/run_all_tests_common.pl control_t "$(CPPGM_TEST_APP)" my "$(TEST)"
	perl ../scripts/compare_results_common.pl control_t ref my "$(TEST)"

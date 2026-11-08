# Optional assembly extension, outside the required cumulative ladder.
include ../scripts/test_run_id.mk
CPPGM_TEST_APP ?= ../dev/cppgm++
LOWIR_APP ?= ../dev/lowir
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_STDLIB_FLAGS ?=
CPPGM_ASSEMBLY_LEVELS ?= O0,O3
export CXX CPPGM_HOST_CXX CPPGM_STDLIB_FLAGS CPPGM_ASSEMBLY_LEVELS
export CPPGM_ASSEMBLY_LOWIR_APP := $(LOWIR_APP)
TEST ?= tests

.PHONY: all test check test-nobuild
all:
	$(MAKE) -C ../dev cppgm++ lowir

test check: all
	$(MAKE) test-nobuild

test-nobuild:
	perl ../scripts/run_all_tests_common.pl control_t "$(CPPGM_TEST_APP)" my "$(TEST)"
	perl ../scripts/compare_results_common.pl control_t ref my "$(TEST)"

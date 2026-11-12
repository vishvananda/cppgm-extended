TARGET = lowir2native
CXX ?= g++
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_TEST_APP ?= ../dev/$(TARGET)
REF_TEST_APP ?= $(CPPGM_TEST_APP)-ref
TEST_DEPS ?= dev-shared-target

.PHONY: all dev-shared-target test ref-test benchmark clean

WASM_TOOLS ?= ../obj/wasm-tools
WASM_BENCH_ARGS ?=

all: dev-shared-target

dev-shared-target:
	@$(MAKE) -C ../dev CXX="$(CXX)" CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" $(TARGET) cppgm++

test: $(TEST_DEPS)
	@perl ../scripts/run_all_tests_common.pl control_t $(CPPGM_TEST_APP) my tests
	@perl ../scripts/compare_results_common.pl control_t ref my tests

ref-test: $(TEST_DEPS)
	@perl ../scripts/run_all_tests_common.pl control_t $(REF_TEST_APP) ref "$(if $(TEST),$(TEST),tests)"
	@perl ../scripts/compare_results_common.pl control_t ref ref "$(if $(TEST),$(TEST),tests)"

CHECK_RUN = perl ../scripts/run_all_tests_common.pl control_t $(CPPGM_TEST_APP) check "$(TEST)"
CHECK_COMPARE = perl ../scripts/compare_results_common.pl control_t ref check "$(TEST)"
include ../scripts/pa_run_check_targets.mk

benchmark: $(TEST_DEPS)
	perl ../scripts/wasm_benchmarks.pl --compiler ../dev/cppgm++ --tools "$(WASM_TOOLS)" \
		--host-cxx "$(CPPGM_HOST_CXX)" $(WASM_BENCH_ARGS)

clean:
	@:

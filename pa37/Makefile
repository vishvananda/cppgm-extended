TARGET = lowir2native
CXX ?= g++
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_TEST_APP ?= ../dev/$(TARGET)
REF_TEST_APP ?= $(CPPGM_TEST_APP)-ref
TEST_DEPS ?= dev-shared-target

.PHONY: all dev-shared-target test ref-test clean

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

clean:
	@:

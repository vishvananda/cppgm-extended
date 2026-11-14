TARGET = cppgm++
TARGET_ARGS = --emit-lowir -O0 -x c -std=c17
export CPPGM_APP_ARGS := $(TARGET_ARGS)
CXX ?= g++
CPPGM_HOST_CXX ?= $(CXX)
CPPGM_TEST_APP ?= ../dev/$(TARGET)
REF_TEST_APP ?= $(CPPGM_TEST_APP)-ref
NATIVE_REFERENCE_APP ?= ../dev/lowir2native-ref
LOWIR2NATIVE_APP ?= ../dev/lowir2native
TEST_DEPS ?= dev-shared-target
GENERAL_TEST_ROOT ?= tests/general
CONTROL_TEST_ROOT ?= tests/controls

.PHONY: all dev-shared-target test test-controls ref-test test-wasm clean
all: dev-shared-target

dev-shared-target:
	@if [ "$(CPPGM_SKIP_DEV_REBUILD)" != "1" ]; then $(MAKE) -C ../dev $(TARGET) lowir2native CXX="$(CXX)" CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)"; fi

CONTROLS_RUN = CPPGM_CONTROL_NATIVE_APP="$(NATIVE_REFERENCE_APP)" \
	perl ../scripts/run_all_tests_common.pl control_t $(CPPGM_TEST_APP) my "$(CONTROL_TEST_ROOT)" && \
	perl ../scripts/compare_results_common.pl control_t ref my "$(CONTROL_TEST_ROOT)"

test: $(TEST_DEPS)
	@scripts/run_all_tests.pl $(CPPGM_TEST_APP) my "$(GENERAL_TEST_ROOT)"
	@scripts/compare_results.pl ref my "$(GENERAL_TEST_ROOT)"
	@$(CONTROLS_RUN)

test-controls: $(TEST_DEPS)
	@$(CONTROLS_RUN)

ref-test:
	@$(REF_TEST_RUN)

REF_TEST_RUN = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) echo "$(TEST): focused LowIR predicate has no reference sidecar" ;; \
	*) scripts/run_all_tests.pl $(REF_TEST_APP) ref "$(if $(strip $(TEST)),$(TEST),$(GENERAL_TEST_ROOT))" ;; \
esac

CHECK_RUN = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) CPPGM_CONTROL_NATIVE_APP="$(NATIVE_REFERENCE_APP)" perl ../scripts/run_all_tests_common.pl control_t $(CPPGM_TEST_APP) check "$(TEST)" ;; \
	*) scripts/run_all_tests.pl $(CPPGM_TEST_APP) check "$(TEST)" ;; \
esac
CHECK_COMPARE = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) perl ../scripts/compare_results_common.pl control_t ref check "$(TEST)" ;; \
	*) scripts/compare_results.pl ref check "$(TEST)" ;; \
esac
include ../scripts/pa_run_check_targets.mk

# The same C source fixtures can also be lowered to Wasm. Expectations state
# results directly; this target does not compare against another C compiler.
test-wasm: $(TEST_DEPS)
	@CPPGM_WASM_CPPGM="$(CPPGM_TEST_APP)" perl ../scripts/run_all_tests_common.pl control_t $(LOWIR2NATIVE_APP) my.wasm "$(GENERAL_TEST_ROOT)"
	@perl ../scripts/compare_results_common.pl control_t ref my.wasm "$(GENERAL_TEST_ROOT)"

clean:
	@rm -f $(TARGET) $(TARGET)-debug

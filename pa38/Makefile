TARGET = lowir2native
LOWIR2NATIVE_APP ?= ../dev/$(TARGET)
LOWIROPT_APP ?= ../dev/lowiropt
CPPGM_DRIVER_APP ?= ../dev/cppgm++
LOWIR2NATIVE_REF_APP ?= $(LOWIR2NATIVE_APP)-ref
TEST_DEPS ?= dev-shared-target
HOST_UNAME_S = $(shell uname -s)
LLVM_CXX_LOCAL = /usr/local/opt/llvm/bin/clang++
LLVM_CXX_HOMEBREW = /opt/homebrew/opt/llvm/bin/clang++
ifeq ($(HOST_UNAME_S),Darwin)
ifeq ($(wildcard $(LLVM_CXX_LOCAL)),)
ifneq ($(wildcard $(LLVM_CXX_HOMEBREW)),)
LLVM_CXX_DEFAULT = $(LLVM_CXX_HOMEBREW)
endif
else
LLVM_CXX_DEFAULT = $(LLVM_CXX_LOCAL)
endif
ifdef LLVM_CXX_DEFAULT
HOST_CXX_DEFAULT = $(LLVM_CXX_DEFAULT)
else
HOST_CXX_DEFAULT = clang++
endif
else
HOST_CXX_DEFAULT = g++
endif

ifeq ($(origin CXX), default)
CXX := $(HOST_CXX_DEFAULT)
endif
ifeq ($(origin CPPGM_HOST_CXX), undefined)
CPPGM_HOST_CXX := $(CXX)
endif
export CPPGM_HOST_CXX
CPPGM_STDLIB_FLAGS ?=
REPORT_EMPTY_OPTIONAL_TEST_ROOTS ?= 0
CC_FLAGS = -std=gnu++11 -Wall -O3 $(CPPGM_STDLIB_FLAGS)
SRC = ../dev/src
GENERATED = ../obj/generated
INC = -I$(SRC) -I$(GENERATED)
OBJ = ../obj
VPATH = $(SRC)
RDEPDIR = $(OBJ)/release/.d
DDEPDIR = $(OBJ)/debug/.d
O1_TEST_ROOT ?= tests/o1
O2_TEST_ROOT ?= tests/o2
O3_TEST_ROOT ?= tests/o3
DEBUGINFO_TEST_ROOT ?= tests/debuginfo
DEBUGINFO_O1_TEST_ROOT ?= $(DEBUGINFO_TEST_ROOT)/o1
DEBUGINFO_O2_TEST_ROOT ?= $(DEBUGINFO_TEST_ROOT)/o2
DEBUGINFO_O3_TEST_ROOT ?= $(DEBUGINFO_TEST_ROOT)/o3
BEHAVIOR_TEST_ROOT ?= tests/behavior
BEHAVIOR_O1_TEST_ROOT ?= $(BEHAVIOR_TEST_ROOT)/o1
BEHAVIOR_O2_TEST_ROOT ?= $(BEHAVIOR_TEST_ROOT)/o2
BEHAVIOR_O3_TEST_ROOT ?= $(BEHAVIOR_TEST_ROOT)/o3
# Our own shape references live in the regression lane, outside the course
# contract (tests/regression; see docs/student-export-root/TESTING_AND_REFERENCES.md).
REGRESSION_O1_TEST_ROOT ?= tests/regression/o1
REGRESSION_O2_TEST_ROOT ?= tests/regression/o2
REGRESSION_O3_TEST_ROOT ?= tests/regression/o3
REGRESSION_BEHAVIOR_O1_TEST_ROOT ?= tests/regression/behavior/o1
REGRESSION_BEHAVIOR_O2_TEST_ROOT ?= tests/regression/behavior/o2
REGRESSION_BEHAVIOR_O3_TEST_ROOT ?= tests/regression/behavior/o3
PERF_TOLERANCE_PERCENT ?= 10
DRIVER_CENSUS_TEST_ROOT ?= tests/driver
STRUCTURAL_CONTROL_TEST_ROOT ?= tests/regression/controls

$(shell mkdir -p $(RDEPDIR) $(DDEPDIR) >/dev/null)

include ../dev/frontend_source_sets.mk

common_obj_basenames = $(FRONTEND_OBJ_BASENAMES_$(TARGET))
common_objs = $(addprefix $(OBJ)/$(1)/,$(addsuffix .o,$(common_obj_basenames)))

all: dev-shared-target

$(OBJ)/release $(OBJ)/debug: %:
	@- mkdir -p $@

$(OBJ)/release/%.o: %.cpp
	@mkdir -p $(@D) $(dir $(RDEPDIR)/$*)
	$(CXX) $(CC_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(RDEPDIR)/$*.Td -o $@ $<
	mv -f $(RDEPDIR)/$*.Td $(RDEPDIR)/$*.d

$(OBJ)/debug/%.o: %.cpp
	@mkdir -p $(@D) $(dir $(DDEPDIR)/$*)
	$(CXX) $(CC_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(DDEPDIR)/$*.Td -o $@ $<
	mv -f $(DDEPDIR)/$*.Td $(DDEPDIR)/$*.d

define run_native_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		CPPGM_APP_ARGS='$(2)' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_APP) my $(1); \
		scripts/compare_results.pl ref my $(1); \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

# A behaviour bucket's references are the program's outcome only; the worker
# leaves no machine IR dump to canonicalize (README: "Behavior tests omit
# x.ref.mir and x.ref.cmir").
define run_native_behavior_bucket_ref
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		CPPGM_NATIVE_BEHAVIOR_ONLY=1 CPPGM_APP_ARGS='$(2)' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_REF_APP) ref $(1); \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

define run_native_bucket_ref
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		CPPGM_APP_ARGS='$(2)' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_REF_APP) ref $(1) && \
		perl ../scripts/canonicalize_lowir_native_refs.pl $(1); \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

define run_driver_census_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		perl ../scripts/check_cppgm_function_census.pl $(CPPGM_DRIVER_APP) "$(1)"; \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

define run_structural_control_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		perl ../scripts/check_lowir_native_structural_controls.pl \
			$(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_native_parameter_rematerialization.pl \
			$(LOWIROPT_APP) $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_native_sibling_tail.pl \
			$(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_native_scalar_query.pl \
			$(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

define run_survivor_property_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		perl ../scripts/check_lowir_native_survivor_properties.pl \
			$(LOWIR2NATIVE_APP) "$(1)"; \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

# The course suite (test-course) judges by behaviour, contract and envelope;
# the regression suite (test-regression) pins the course solution's dumps and
# its placement controls.  test-variants runs the course suite under each
# design variant of dev/src/backend_variant.h.
test: test-course test-regression

test-course: $(TEST_DEPS)
	$(call run_native_bucket,$(O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(O3_TEST_ROOT),-O3)
	$(call run_native_bucket,$(BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(BEHAVIOR_O3_TEST_ROOT),-O3)
	$(call run_driver_census_bucket,$(DRIVER_CENSUS_TEST_ROOT))

test-regression: $(TEST_DEPS)
	$(call run_native_bucket,$(REGRESSION_O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(REGRESSION_O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(REGRESSION_O3_TEST_ROOT),-O3)
	$(call run_native_bucket,$(REGRESSION_BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(REGRESSION_BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(REGRESSION_BEHAVIOR_O3_TEST_ROOT),-O3)
	$(call run_structural_control_bucket,$(STRUCTURAL_CONTROL_TEST_ROOT))
	$(call run_survivor_property_bucket,$(REGRESSION_O1_TEST_ROOT))
	$(call run_survivor_property_bucket,$(REGRESSION_O2_TEST_ROOT))
	$(call run_survivor_property_bucket,$(REGRESSION_BEHAVIOR_O1_TEST_ROOT))

BACKEND_VARIANTS ?= pool-reverse frame-pad rpo-layout colouring linear-scan
test-variants: $(TEST_DEPS)
	@set -e; for variant in $(BACKEND_VARIANTS); do \
		echo "== $(TARGET) course suite under variant $$variant"; \
		CPPGM_BACKEND_VARIANT=$$variant $(MAKE) --no-print-directory test-course; \
	done

test-debuginfo: $(TEST_DEPS)
	$(call run_native_bucket,$(DEBUGINFO_O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(DEBUGINFO_O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(DEBUGINFO_O3_TEST_ROOT),-O3)

ref-test:
ifneq ($(strip $(TEST)),)
	@$(REF_TEST_RUN)
else
	$(call run_native_bucket_ref,$(O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(O3_TEST_ROOT),-O3)
	$(call run_native_behavior_bucket_ref,$(BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_behavior_bucket_ref,$(BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_behavior_bucket_ref,$(BEHAVIOR_O3_TEST_ROOT),-O3)
	$(call run_native_bucket_ref,$(REGRESSION_O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(REGRESSION_O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(REGRESSION_O3_TEST_ROOT),-O3)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O3_TEST_ROOT),-O3)
endif

ref-test-debuginfo:
	$(call run_native_bucket_ref,$(DEBUGINFO_O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(DEBUGINFO_O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(DEBUGINFO_O3_TEST_ROOT),-O3)

REF_TEST_RUN = case "$(TEST)" in \
	$(DRIVER_CENSUS_TEST_ROOT)/*) echo "$(TEST): structural function-census predicate has no reference sidecar" ;; \
	$(STRUCTURAL_CONTROL_TEST_ROOT)/*) echo "$(TEST): structural native predicate has no reference sidecar" ;; \
	$(O1_TEST_ROOT)/*|$(DEBUGINFO_O1_TEST_ROOT)/*|$(BEHAVIOR_O1_TEST_ROOT)/*) CPPGM_APP_ARGS='-O1' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_REF_APP) ref "$(TEST)" && perl ../scripts/canonicalize_lowir_native_refs.pl "$(TEST)" ;; \
	$(O2_TEST_ROOT)/*|$(DEBUGINFO_O2_TEST_ROOT)/*|$(BEHAVIOR_O2_TEST_ROOT)/*) CPPGM_APP_ARGS='-O2' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_REF_APP) ref "$(TEST)" && perl ../scripts/canonicalize_lowir_native_refs.pl "$(TEST)" ;; \
	$(O3_TEST_ROOT)/*|$(DEBUGINFO_O3_TEST_ROOT)/*|$(BEHAVIOR_O3_TEST_ROOT)/*) CPPGM_APP_ARGS='-O3' perl ../scripts/run_lowir_native_tests_worker.pl $(LOWIR2NATIVE_REF_APP) ref "$(TEST)" && perl ../scripts/canonicalize_lowir_native_refs.pl "$(TEST)" ;; \
	*) echo "Usage: make ref-test TEST=$(O1_TEST_ROOT)/<file>.t | $(O2_TEST_ROOT)/<file>.t | $(O3_TEST_ROOT)/<file>.t"; exit 1 ;; \
esac


CHECK_RUN = perl ../scripts/run_routed_test_spec.pl "$(TEST)" \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-scalar-sibling-query*.t::perl ../scripts/check_lowir_native_scalar_query.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-sibling-tail-transfer*.t::perl ../scripts/check_lowir_native_sibling_tail.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(DRIVER_CENSUS_TEST_ROOT)/*::perl ../scripts/check_cppgm_function_census.pl $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-parameter-address-rematerialization*.t::perl ../scripts/check_lowir_native_parameter_rematerialization.pl $(LOWIROPT_APP) $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*::perl ../scripts/check_lowir_native_structural_controls.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(O1_TEST_ROOT)/*|$(DEBUGINFO_O1_TEST_ROOT)/*|$(BEHAVIOR_O1_TEST_ROOT)/*::CPPGM_APP_ARGS=-O1 scripts/run_all_tests.pl $(LOWIR2NATIVE_APP) check {tests}' \
	'$(O2_TEST_ROOT)/*|$(DEBUGINFO_O2_TEST_ROOT)/*|$(BEHAVIOR_O2_TEST_ROOT)/*::CPPGM_APP_ARGS=-O2 scripts/run_all_tests.pl $(LOWIR2NATIVE_APP) check {tests}' \
	'$(O3_TEST_ROOT)/*|$(DEBUGINFO_O3_TEST_ROOT)/*|$(BEHAVIOR_O3_TEST_ROOT)/*::CPPGM_APP_ARGS=-O3 scripts/run_all_tests.pl $(LOWIR2NATIVE_APP) check {tests}'
CHECK_COMPARE = perl ../scripts/run_routed_test_spec.pl "$(TEST)" \
	'$(DRIVER_CENSUS_TEST_ROOT)/*::true' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*::true' \
	'$(O1_TEST_ROOT)/*|$(DEBUGINFO_O1_TEST_ROOT)/*|$(BEHAVIOR_O1_TEST_ROOT)/*|$(O2_TEST_ROOT)/*|$(DEBUGINFO_O2_TEST_ROOT)/*|$(BEHAVIOR_O2_TEST_ROOT)/*|$(O3_TEST_ROOT)/*|$(DEBUGINFO_O3_TEST_ROOT)/*|$(BEHAVIOR_O3_TEST_ROOT)/*::scripts/compare_results.pl ref check {tests}'
include ../scripts/pa_run_check_targets.mk

.PHONY: dev-shared-target ref-test ref-test-debuginfo test-debuginfo
../dev/$(TARGET):
	$(MAKE) -C ../dev $(TARGET) CXX=$(CXX) CPPGM_HOST_CXX=$(CPPGM_HOST_CXX)

dev-shared-target:
	@if [ "$(CPPGM_SKIP_DEV_REBUILD)" = "1" ]; then true; else $(MAKE) -C ../dev $(TARGET) lowiropt cppgm++ CXX=$(CXX) CPPGM_HOST_CXX=$(CPPGM_HOST_CXX); fi

clean:
	-rm -f $(TARGET) $(TARGET)-debug $(OBJ)/release/*.o $(OBJ)/debug/*.o

$(RDEPDIR)/%.d: ;
.PRECIOUS: $(RDEPDIR)/%.d

$(DDEPDIR)/%.d: ;
.PRECIOUS: $(DDEPDIR)/%.d

-include $(addprefix $(RDEPDIR)/,$(addsuffix .d,$(common_obj_basenames)))
-include $(addprefix $(DDEPDIR)/,$(addsuffix .d,$(common_obj_basenames)))

# The performance envelope: each behaviour program runs under Cachegrind and
# its instruction count must stay within PERF_TOLERANCE_PERCENT of the
# `x.ref.ir` sidecar.  `make test-perf-refs` writes the sidecars from the
# current programs.  Skipped when valgrind is not installed.
test-perf: $(TEST_DEPS)
	$(call run_native_bucket,$(BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_bucket,$(BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_bucket,$(BEHAVIOR_O3_TEST_ROOT),-O3)
	@perl ../scripts/check_ir_envelope.pl $(PERF_TOLERANCE_PERCENT) $(BEHAVIOR_O1_TEST_ROOT) $(BEHAVIOR_O2_TEST_ROOT) $(BEHAVIOR_O3_TEST_ROOT)

test-perf-refs: $(TEST_DEPS)
	@perl ../scripts/check_ir_envelope.pl --write $(PERF_TOLERANCE_PERCENT) $(BEHAVIOR_O1_TEST_ROOT) $(BEHAVIOR_O2_TEST_ROOT) $(BEHAVIOR_O3_TEST_ROOT)

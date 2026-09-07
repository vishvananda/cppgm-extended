# Solution regression and profiling lanes; omitted from the student export.
LOWIROPT_APP ?= ../dev/lowiropt
REGRESSION_O1_TEST_ROOT ?= tests/regression/o1
REGRESSION_O2_TEST_ROOT ?= tests/regression/o2
REGRESSION_O3_TEST_ROOT ?= tests/regression/o3
REGRESSION_BEHAVIOR_O1_TEST_ROOT ?= tests/regression/behavior/o1
REGRESSION_BEHAVIOR_O2_TEST_ROOT ?= tests/regression/behavior/o2
REGRESSION_BEHAVIOR_O3_TEST_ROOT ?= tests/regression/behavior/o3
PERF_TOLERANCE_PERCENT ?= 10
DRIVER_CENSUS_TEST_ROOT ?= tests/regression/driver
STRUCTURAL_CONTROL_TEST_ROOT ?= tests/regression/controls

define run_driver_census_bucket
	@if [ -d "$(1)" ] && find "$(1)" \( -type f -o -type l \) -name '*.t' | grep -q .; then \
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

test: test-regression

test-regression: $(TEST_DEPS)
	$(call run_driver_census_bucket,$(DRIVER_CENSUS_TEST_ROOT))
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
		CPPGM_BACKEND_VARIANT=$$variant $(MAKE) --no-print-directory test-course test-debuginfo; \
	done

ifeq ($(strip $(TEST)),)
ref-test: ref-test-regression
endif

ref-test-regression:
	$(call run_native_bucket_ref,$(REGRESSION_O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(REGRESSION_O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(REGRESSION_O3_TEST_ROOT),-O3)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O1_TEST_ROOT),-O1)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O2_TEST_ROOT),-O2)
	$(call run_native_bucket_ref,$(REGRESSION_BEHAVIOR_O3_TEST_ROOT),-O3)

CHECK_RUN += \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-scalar-sibling-query*.t::perl ../scripts/check_lowir_native_scalar_query.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-sibling-tail-transfer*.t::perl ../scripts/check_lowir_native_sibling_tail.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(DRIVER_CENSUS_TEST_ROOT)/*::perl ../scripts/check_cppgm_function_census.pl $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*o3-parameter-address-rematerialization*.t::perl ../scripts/check_lowir_native_parameter_rematerialization.pl $(LOWIROPT_APP) $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*::perl ../scripts/check_lowir_native_structural_controls.pl $(LOWIR2NATIVE_APP) $(CPPGM_DRIVER_APP) {tests}'

CHECK_COMPARE += \
	'$(DRIVER_CENSUS_TEST_ROOT)/*::true' \
	'$(STRUCTURAL_CONTROL_TEST_ROOT)/*::true'

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

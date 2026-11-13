# Solution regression lanes; omitted from the student export.
#
# tests/regression pins this solution's exact LowIR, and its controls check
# this solution's pass-specific choices, outside the course contract. One
# fixture's reference is regenerated with the course root pointed at its lane:
#   make ref-test TEST=tests/regression/o3/x.t O3_TEST_ROOT=tests/regression/o3
REGRESSION_O0_TEST_ROOT ?= tests/regression/o0
REGRESSION_O1_TEST_ROOT ?= tests/regression/o1
REGRESSION_O2_TEST_ROOT ?= tests/regression/o2
REGRESSION_O3_TEST_ROOT ?= tests/regression/o3
REGRESSION_DRIVER_O1_TEST_ROOT ?= tests/regression/driver/o1
REGRESSION_DRIVER_O2_TEST_ROOT ?= tests/regression/driver/o2
REGRESSION_DRIVER_O3_TEST_ROOT ?= tests/regression/driver/o3
CONTROL_TEST_ROOT ?= tests/regression/controls

define run_inline_control_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		set -e; \
		perl ../scripts/check_lowir_inline_limit.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_boolean_phi_controls.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_zero_bounded_range.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_addressed_scalar_slots.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_inlined_addressed_store.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_post_inline_promotion.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_historical_contracts.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_late_load_reuse.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_weak_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_o3_group_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_readonly_string_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_o3_loop_priority_inline.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_terminal_phi_returns.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_readonly_strlen.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_copy_elision.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_terminal_object_swap.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_bounded_object_memory.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_fast_path_versioning.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) "$(1)"; \
		perl ../scripts/check_lowir_survivor_properties.pl $(LOWIROPT_APP) "$(1)"; \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

define run_survivor_property_bucket
	@if [ -d "$(1)" ] && find "$(1)" -type f -name '*.t' | grep -q .; then \
		perl ../scripts/check_lowir_survivor_properties.pl \
			$(LOWIROPT_APP) "$(1)"; \
	else \
		if [ "$(REPORT_EMPTY_OPTIONAL_TEST_ROOTS)" = "1" ]; then echo "$(TARGET) $(1): no tests"; fi; \
	fi
endef

test: test-regression

test-regression: $(TEST_DEPS)
	$(call run_text_course_bucket,-O0,$(LOWIROPT_APP),$(REGRESSION_O0_TEST_ROOT))
	$(call run_text_course_bucket,-O1,$(LOWIROPT_APP),$(REGRESSION_O1_TEST_ROOT))
	$(call run_text_course_bucket,-O2,$(LOWIROPT_APP),$(REGRESSION_O2_TEST_ROOT))
	$(call run_text_course_bucket,-O3,$(LOWIROPT_APP),$(REGRESSION_O3_TEST_ROOT))
	$(call run_text_course_bucket,--emit-lowir -g0 -O1,$(CPPGM_DRIVER_APP),$(REGRESSION_DRIVER_O1_TEST_ROOT))
	$(call run_text_course_bucket,--emit-lowir -g0 -O2,$(CPPGM_DRIVER_APP),$(REGRESSION_DRIVER_O2_TEST_ROOT))
	$(call run_text_course_bucket,--emit-lowir -g0 -O3,$(CPPGM_DRIVER_APP),$(REGRESSION_DRIVER_O3_TEST_ROOT))
	$(call run_inline_control_bucket,$(CONTROL_TEST_ROOT))
	$(call run_survivor_property_bucket,$(REGRESSION_O1_TEST_ROOT))
	$(call run_survivor_property_bucket,$(REGRESSION_O2_TEST_ROOT))

ifeq ($(strip $(TEST)),)
ref-test: ref-test-regression
endif

ref-test-regression:
	@if [ -d "$(REGRESSION_O0_TEST_ROOT)" ]; then CPPGM_APP_ARGS='-O0' scripts/run_all_tests.pl $(LOWIROPT_REF_APP) ref $(REGRESSION_O0_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_O1_TEST_ROOT)" ]; then CPPGM_APP_ARGS='-O1' scripts/run_all_tests.pl $(LOWIROPT_REF_APP) ref $(REGRESSION_O1_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_O2_TEST_ROOT)" ]; then CPPGM_APP_ARGS='-O2' scripts/run_all_tests.pl $(LOWIROPT_REF_APP) ref $(REGRESSION_O2_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_O3_TEST_ROOT)" ]; then CPPGM_APP_ARGS='-O3' scripts/run_all_tests.pl $(LOWIROPT_REF_APP) ref $(REGRESSION_O3_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_DRIVER_O1_TEST_ROOT)" ]; then CPPGM_APP_ARGS='--emit-lowir -g0 -O1' scripts/run_all_tests.pl $(CPPGM_DRIVER_REF_APP) ref $(REGRESSION_DRIVER_O1_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_DRIVER_O2_TEST_ROOT)" ]; then CPPGM_APP_ARGS='--emit-lowir -g0 -O2' scripts/run_all_tests.pl $(CPPGM_DRIVER_REF_APP) ref $(REGRESSION_DRIVER_O2_TEST_ROOT); fi
	@if [ -d "$(REGRESSION_DRIVER_O3_TEST_ROOT)" ]; then CPPGM_APP_ARGS='--emit-lowir -g0 -O3' scripts/run_all_tests.pl $(CPPGM_DRIVER_REF_APP) ref $(REGRESSION_DRIVER_O3_TEST_ROOT); fi

CHECK_RUN += \
	'$(CONTROL_TEST_ROOT)/*o3-fast-path-versioning*.t::perl ../scripts/check_lowir_fast_path_versioning.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*o3-bounded-object-memory*.t::perl ../scripts/check_lowir_bounded_object_memory.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*o3-terminal-staged-object-swap*.t::perl ../scripts/check_lowir_terminal_object_swap.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*copy-elision-permission*.t::perl ../scripts/check_lowir_copy_elision.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*loop-local-boolean-phi-branch*.t::perl ../scripts/check_lowir_boolean_phi_controls.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*zero-bounded-signed-range*.t::perl ../scripts/check_lowir_zero_bounded_range.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*inlined-addressed-store*.t::perl ../scripts/check_lowir_inlined_addressed_store.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*post-prune-inline-slot-promotion*.t::perl ../scripts/check_lowir_post_inline_promotion.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*historical-lowir-contracts*.t::perl ../scripts/check_lowir_historical_contracts.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*late-inline-hint-load-reuse*.t::perl ../scripts/check_lowir_late_load_reuse.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*post-inline-memory-gvn*.t::perl ../scripts/check_lowir_late_load_reuse.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*weak-specialization-profitability*.t::perl ../scripts/check_lowir_weak_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*o3-group-specialization*.t::perl ../scripts/check_lowir_o3_group_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*readonly-string-specialization*.t::perl ../scripts/check_lowir_readonly_string_specialization.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*o3-loop-priority-inline*.t::perl ../scripts/check_lowir_o3_loop_priority_inline.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*o3-terminal-phi-return*.t::perl ../scripts/check_lowir_terminal_phi_returns.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*readonly-byte-strlen*.t::perl ../scripts/check_lowir_readonly_strlen.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*inline-limit*.t::perl ../scripts/check_lowir_inline_limit.pl $(LOWIROPT_APP) $(CPPGM_DRIVER_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*nonzero-underflow-value-proof*.t::perl ../scripts/check_lowir_survivor_properties.pl $(LOWIROPT_APP) {tests}' \
	'$(CONTROL_TEST_ROOT)/*loop-carried-store-forwarding*.t::perl ../scripts/check_lowir_survivor_properties.pl $(LOWIROPT_APP) {tests}'

CHECK_COMPARE += \
	'$(CONTROL_TEST_ROOT)/*::true'

.PHONY: test-regression ref-test-regression

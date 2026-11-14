# Solution regression lane; omitted from the student export.
#
# tests/regression bounds this solution's instruction selection (the views of
# narrow values, memory offsets, object copies, the places of by-value objects
# and the frame's stack test) and its linked memory layout, outside the course
# contract. One fixture runs alone with:
#   make check TEST=tests/regression/x.t
REGRESSION_TEST_ROOT ?= tests/regression

test: test-regression

test-regression: $(TEST_DEPS)
	@perl ../scripts/run_all_tests_common.pl control_t $(CPPGM_TEST_APP) my $(REGRESSION_TEST_ROOT)
	@perl ../scripts/compare_results_common.pl control_t ref my $(REGRESSION_TEST_ROOT)

# Without TEST, `make ref-test` regenerates the regression lane's references
# as well; the course runner prunes `regression/` from `tests`.
ifeq ($(strip $(TEST)),)
ref-test: ref-test-regression
endif

ref-test-regression: $(TEST_DEPS)
	@perl ../scripts/run_all_tests_common.pl control_t $(REF_TEST_APP) ref $(REGRESSION_TEST_ROOT)
	@perl ../scripts/compare_results_common.pl control_t ref ref $(REGRESSION_TEST_ROOT)

.PHONY: test-regression ref-test-regression

# Solution regression lanes; omitted from the student export.
#
# tests/regression pins this solution's machine IR: `strict/` compares the
# dump after target header normalization, `structural/` its canonical form and
# `behavior/` the program results. The controls check its selection policies
# (its copy and zero thresholds, frame-pointer and epilogue choice, switch
# compare chain, carried reloads and parameter carriers), outside the course
# contract. One control runs with the course root pointed at the lane:
#   make check TEST=tests/regression/controls/x.t CONTROL_TEST_ROOT=tests/regression/controls
REGRESSION_ORACLE_TEST_ROOTS = $(wildcard tests/regression/strict tests/regression/structural tests/regression/behavior)
REGRESSION_CONTROL_TEST_ROOT = tests/regression/controls
REGRESSION_CONTROL_BEHAVIOR_TESTS = \
	tests/behavior/900-scratch-carried-frame-reloads.t \
	tests/behavior/900-deferred-address-parameter-carrier-reuse.t \
	tests/behavior/800-cost-directed-small-zeroinit.t

test: test-regression

test-regression: $(TEST_DEPS)
	@set -e; if [ -d "$(REGRESSION_CONTROL_TEST_ROOT)" ]; then \
		perl ../scripts/check_pa24_native_contracts.pl \
			$(CPPGM_TEST_APP) "$(REGRESSION_CONTROL_TEST_ROOT)"; \
	fi; \
	for test in $(REGRESSION_CONTROL_BEHAVIOR_TESTS); do \
		perl ../scripts/check_pa24_native_contracts.pl $(CPPGM_TEST_APP) "$$test"; \
	done
ifneq ($(CPPGM_BATCH_TESTS),)
	@set -e; for root in $(REGRESSION_ORACLE_TEST_ROOTS); do \
		perl ../scripts/run_lowir_native_tests_worker.pl \
			$(CPPGM_TEST_APP) my "$$root"; \
		case "$$root" in \
			*/behavior) mode=mir_behavior_t ;; \
			*/structural) mode=mir_structural_t ;; \
			*) mode=mir_t ;; \
		esac; \
		perl ../scripts/compare_results_common.pl \
			"$$mode" ref my "$$root"; \
	done
else
	@set -e; for root in $(REGRESSION_ORACLE_TEST_ROOTS); do \
		scripts/run_all_tests.pl $(CPPGM_TEST_APP) my "$$root"; \
		case "$$root" in \
			*/behavior) mode=mir_behavior_t ;; \
			*/structural) mode=mir_structural_t ;; \
			*) mode=mir_t ;; \
		esac; \
		perl ../scripts/compare_results_common.pl \
			"$$mode" ref my "$$root"; \
	done
endif

# Without TEST, `make ref-test` regenerates the regression lanes' references
# as well; the course runner prunes `regression/` from `tests`. A structural
# reference carries a canonical `.ref.cmir`, so the lane's root is
# recanonicalized with the course's.
STRUCTURAL_TEST_ROOTS += $(wildcard tests/regression/structural)

ifeq ($(strip $(TEST)),)
ref-test: ref-test-regression
endif

ref-test-regression:
	@set -e; for root in $(REGRESSION_ORACLE_TEST_ROOTS); do \
		scripts/run_all_tests.pl $(REF_TEST_APP) ref "$$root"; \
	done
	@if [ -d tests/regression/structural ]; then \
		perl ../scripts/canonicalize_lowir_native_refs.pl tests/regression/structural; \
	fi

.PHONY: test-regression ref-test-regression

TARGET = lowir2native
CXX = g++
CPPGM_STDLIB_FLAGS ?=
CPPGM_TEST_APP ?= ../dev/$(TARGET)
REF_TEST_APP ?= $(CPPGM_TEST_APP)-ref
TEST_DEPS ?= dev-shared-target
CC_FLAGS = -std=gnu++11 -Wall -O3 $(CPPGM_STDLIB_FLAGS)
SRC = ../dev/src
GENERATED = ../obj/generated
INC = -I$(SRC) -I$(GENERATED)
OBJ = ../obj
VPATH = $(SRC)
RDEPDIR = $(OBJ)/release/.d
DDEPDIR = $(OBJ)/debug/.d
CONTROL_TEST_ROOT ?= tests/controls
PERF_TOLERANCE_PERCENT ?= 10

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

# The course suite (test-course) judges by behaviour, contract shape where
# the shape is the contract, and envelope.
test: test-course

test-course: $(TEST_DEPS)
ifneq ($(CPPGM_BATCH_TESTS),)
	@perl ../scripts/run_lowir_native_tests_worker.pl $(CPPGM_TEST_APP) my tests
else
	@scripts/run_all_tests.pl $(CPPGM_TEST_APP) my tests
endif
	@scripts/compare_results.pl ref my tests
	@set -e; if [ -d "$(CONTROL_TEST_ROOT)" ]; then \
		perl ../scripts/check_pa24_native_contracts.pl \
			$(CPPGM_TEST_APP) "$(CONTROL_TEST_ROOT)"; \
	fi

# Structural references carry a canonical `.ref.cmir` beside the dump, so
# every structural root is recanonicalized, whichever fixtures were
# regenerated.
STRUCTURAL_TEST_ROOTS = $(wildcard tests/structural)

ref-test:
	@$(REF_TEST_RUN)
	@set -e; for root in $(STRUCTURAL_TEST_ROOTS); do \
		perl ../scripts/canonicalize_lowir_native_refs.pl "$$root"; \
	done

REF_TEST_RUN = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) echo "$(TEST): focused native predicate has no reference sidecar" ;; \
	*) scripts/run_all_tests.pl $(REF_TEST_APP) ref "$(if $(strip $(TEST)),$(TEST),tests)" ;; \
esac

CHECK_RUN = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) perl ../scripts/check_pa24_native_contracts.pl $(CPPGM_TEST_APP) "$(TEST)" ;; \
	*) scripts/run_all_tests.pl $(CPPGM_TEST_APP) check "$(TEST)" ;; \
esac
CHECK_COMPARE = case "$(TEST)" in \
	$(CONTROL_TEST_ROOT)/*) true ;; \
	*) scripts/compare_results.pl ref check "$(TEST)" ;; \
esac

include ../scripts/pa_run_check_targets.mk

.PHONY: dev-shared-target ref-test
$(CPPGM_TEST_APP):
	$(MAKE) -C ../dev $(TARGET) CXX=$(CXX) OBJ=$(OBJ)

dev-shared-target:
	@if [ "$(CPPGM_SKIP_DEV_REBUILD)" = "1" ]; then true; else $(MAKE) -C ../dev $(TARGET) CXX=$(CXX) OBJ=$(OBJ); fi

clean:
	-rm -f $(TARGET) $(TARGET)-debug $(OBJ)/release/*.o $(OBJ)/debug/*.o

$(RDEPDIR)/%.d: ;
.PRECIOUS: $(RDEPDIR)/%.d

$(DDEPDIR)/%.d: ;
.PRECIOUS: $(DDEPDIR)/%.d

-include $(addprefix $(RDEPDIR)/,$(addsuffix .d,$(common_obj_basenames)))
-include $(addprefix $(DDEPDIR)/,$(addsuffix .d,$(common_obj_basenames)))

-include maintainer/Makefile

# The performance envelope: each behaviour program runs under Cachegrind and
# its instruction count must stay within PERF_TOLERANCE_PERCENT of the
# `x.ref.ir` sidecar.  `make test-perf-refs` writes the sidecars from the
# current programs.  Skipped when valgrind is not installed.
test-perf: $(TEST_DEPS)
	@perl ../scripts/check_ir_envelope.pl $(PERF_TOLERANCE_PERCENT) tests/behavior

test-perf-refs: $(TEST_DEPS)
	@perl ../scripts/check_ir_envelope.pl --write $(PERF_TOLERANCE_PERCENT) tests/behavior

# Optional Intel macOS target; the required course target remains Linux.
.PHONY: test-macho
test-macho: $(TEST_DEPS)
	python3 ../scripts/check_macho.py --mode image --fixtures tests/macho \
		--compiler ../dev/cppgm++ --lowir2native ../dev/lowir2native \
		--host-cxx "$(CPPGM_HOST_CXX)"

ifeq ($(shell uname -s),Darwin)
test: test-macho
endif

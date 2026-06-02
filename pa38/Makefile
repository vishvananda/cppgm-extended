TARGETS = \
	pptoken-self \
	posttoken-self \
	ctrlexpr-self \
	macro-self \
	preproc-self \
	recog-self \
	nsdecl-self \
	nsinit-self \
	cy86-self \
	lowiropt-self \
	lowir2cy86-self \
	lowir2native-self \
	cppgm++-self

DEFAULT_BUILD_JOBS = $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
HOST_UNAME_S = $(shell uname -s)
ifeq ($(findstring -j,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(DEFAULT_BUILD_JOBS)
endif

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
ifeq ($(abspath $(CXX)),$(abspath ../dev/cppgm++))
CPPGM_HOST_CXX := $(HOST_CXX_DEFAULT)
else
CPPGM_HOST_CXX := $(CXX)
endif
endif

CPPGM_TEST_RUNNER ?= 1
TEST_RUNNER_SHARED_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-DTEST_RUNNER_ENABLE,)
TEST_RUNNER_ENTRY_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-Dmain=test_runner_real_main,)
CPPGM_STDLIB_FLAGS ?=
CPPGM_TEXT_TEST_TIMEOUT_SEC ?= 30
export CPPGM_TEST_JOBS
export CPPGM_HOST_CXX
export CPPGM_STDLIB_FLAGS
export CPPGM_TEXT_TEST_TIMEOUT_SEC
PA38_SELFHOST_OPT_LEVEL ?= 3
PA38_OBJ_ROOT_BASE ?= ../obj/pa38
PA38_BIN_ROOT_BASE ?= $(PA38_OBJ_ROOT_BASE)/bin
PA38_GENERATED ?= $(PA38_OBJ_ROOT_BASE)/generated
PA38_BUILTIN_HOST_CONFIG = $(PA38_GENERATED)/cppgm_builtin_host_config.h
PA38_INCLUDE_PATHS := ../dev/src $(PA38_GENERATED)
PA38_DEFINES ?=
HOST_CXX_DEFAULT_DEF = -DCPPGM_DEFAULT_HOST_CXX='"$(CPPGM_HOST_CXX)"'
OBJECT_ROOT_DEFAULT_DEF = -DCPPGM_DEFAULT_OBJECT_ROOT='"$(abspath $(PA38_OBJ_ROOT_BASE))"'
PA38_CC_FLAGS = -std=gnu++11 -Wall -O$(PA38_SELFHOST_OPT_LEVEL) $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)
PA38_ENTRY_CC_FLAGS = $(PA38_CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
PA38_LINKER_DETERMINISM_FLAGS =
ifeq ($(HOST_UNAME_S),Darwin)
PA38_LINKER_DETERMINISM_FLAGS += -Wl,-reproducible
endif
PA38_LINK_FLAGS = -std=gnu++11 $(CPPGM_STDLIB_FLAGS) $(PA38_LINKER_DETERMINISM_FLAGS)
include ../dev/frontend_source_sets.mk

ifeq ($(abspath $(CXX)),$(abspath ../dev/cppgm++))
PA38_COMPILER_FLAVOR ?= selfhost
else
PA38_COMPILER_FLAVOR ?= host
endif

ifneq ($(filter ./% ../% /%,$(CXX)),)
PA38_CXX_DEP := $(CXX)
else
PA38_CXX_DEP :=
endif

PA38_OBJ_ROOT ?= $(PA38_OBJ_ROOT_BASE)/$(PA38_COMPILER_FLAVOR)
PA38_BIN_ROOT ?= $(PA38_BIN_ROOT_BASE)/$(PA38_COMPILER_FLAVOR)
PA38_OUTPUT_SUFFIX ?= -self

CHECKPOINTS = pptoken posttoken ctrlexpr macro preproc recog nsdecl nsinit cy86 cppgm++ lowiropt lowir2cy86 lowir2native
TEST_TARGETS = $(addprefix test-,$(CHECKPOINTS))
THROUGH_TARGETS = $(addprefix through-,$(CHECKPOINTS))
TEST_THROUGH_TARGETS = $(addprefix test-through-,$(CHECKPOINTS))
INCEPTION_TARGETS = $(addsuffix -inception,$(CHECKPOINTS))
INCEPTION_COMPARE_TARGETS = $(addprefix compare-,$(INCEPTION_TARGETS))
PA_STAGE_NAMES = pa1 pa2 pa3 pa4 pa5 pa6 pa7 pa8 pa9 pa10 pa11 pa12 pa13 pa14 pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22 pa24 pa23 pa25 pa26 pa27 pa28 pa29 pa30 pa32 pa33 pa34 pa35 pa36 pa37
PA38_BATCH_TEST_STAGE_NAMES = pa10 pa11 pa12 pa13 pa14 pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22 pa24 pa23 pa25 pa26 pa27 pa28 pa29 pa30 pa32 pa33 pa34 pa35 pa36 pa37
PA38_BATCH_TEST_ENV = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),CPPGM_BATCH_TESTS=1 WRAPPED_BATCH_STDIN=1,)
pa38_stage_test_env = $(if $(filter $(1),$(PA38_BATCH_TEST_STAGE_NAMES)),$(PA38_BATCH_TEST_ENV),)
PA_THROUGH_TARGETS = $(addprefix through-,$(PA_STAGE_NAMES))
PA_TEST_THROUGH_TARGETS = $(addprefix test-through-,$(PA_STAGE_NAMES))
PA_TEST_TARGETS = $(addprefix test-,$(PA_STAGE_NAMES))
PA_TEST_NOBUILD_TARGETS = $(addsuffix -nobuild,$(PA_TEST_TARGETS))

PA38_PREV_pptoken =
PA38_PREV_posttoken = pptoken
PA38_PREV_ctrlexpr = posttoken
PA38_PREV_macro = ctrlexpr
PA38_PREV_preproc = macro
PA38_PREV_recog = preproc
PA38_PREV_nsdecl = recog
PA38_PREV_nsinit = nsdecl
PA38_PREV_cy86 = nsinit
PA38_PREV_cppgm++ = cy86
PA38_PREV_lowiropt = cppgm++
PA38_PREV_lowir2cy86 = cppgm++
PA38_PREV_lowir2native = lowir2cy86

PA38_CHECKPOINT_FOR_pa1 = pptoken
PA38_CHECKPOINT_FOR_pa2 = posttoken
PA38_CHECKPOINT_FOR_pa3 = ctrlexpr
PA38_CHECKPOINT_FOR_pa4 = macro
PA38_CHECKPOINT_FOR_pa5 = preproc
PA38_CHECKPOINT_FOR_pa6 = recog
PA38_CHECKPOINT_FOR_pa7 = nsdecl
PA38_CHECKPOINT_FOR_pa8 = nsinit
PA38_CHECKPOINT_FOR_pa9 = cy86
PA38_CHECKPOINT_FOR_pa10 = cppgm++
PA38_CHECKPOINT_FOR_pa11 = cppgm++
PA38_CHECKPOINT_FOR_pa12 = cppgm++
PA38_CHECKPOINT_FOR_pa13 = lowir2cy86
PA38_CHECKPOINT_FOR_pa14 = cppgm++
PA38_CHECKPOINT_FOR_pa15 = cppgm++
PA38_CHECKPOINT_FOR_pa16 = cppgm++
PA38_CHECKPOINT_FOR_pa17 = cppgm++
PA38_CHECKPOINT_FOR_pa18 = cppgm++
PA38_CHECKPOINT_FOR_pa19 = cppgm++
PA38_CHECKPOINT_FOR_pa20 = cppgm++
PA38_CHECKPOINT_FOR_pa21 = cppgm++
PA38_CHECKPOINT_FOR_pa22 = cppgm++
PA38_CHECKPOINT_FOR_pa24 = cppgm++
PA38_CHECKPOINT_FOR_pa23 = lowir2native
PA38_CHECKPOINT_FOR_pa25 = cppgm++
PA38_CHECKPOINT_FOR_pa26 = cppgm++
PA38_CHECKPOINT_FOR_pa27 = cppgm++
PA38_CHECKPOINT_FOR_pa28 = cppgm++
PA38_CHECKPOINT_FOR_pa29 = cppgm++
PA38_CHECKPOINT_FOR_pa30 = cppgm++
PA38_CHECKPOINT_FOR_pa32 = cppgm++
PA38_CHECKPOINT_FOR_pa33 = cppgm++
PA38_CHECKPOINT_FOR_pa34 = cppgm++
PA38_CHECKPOINT_FOR_pa35 = cppgm++
PA38_CHECKPOINT_FOR_pa36 = lowiropt
PA38_CHECKPOINT_FOR_pa37 = lowir2native
PA38_PRIMARY_STAGE_pptoken = pa1
PA38_PRIMARY_STAGE_posttoken = pa2
PA38_PRIMARY_STAGE_ctrlexpr = pa3
PA38_PRIMARY_STAGE_macro = pa4
PA38_PRIMARY_STAGE_preproc = pa5
PA38_PRIMARY_STAGE_recog = pa6
PA38_PRIMARY_STAGE_nsdecl = pa7
PA38_PRIMARY_STAGE_nsinit = pa8
PA38_PRIMARY_STAGE_cy86 = pa9
PA38_PRIMARY_STAGE_cppgm++ = pa10
PA38_PRIMARY_STAGE_lowiropt = pa36
PA38_PRIMARY_STAGE_lowir2cy86 = pa13
PA38_PRIMARY_STAGE_lowir2native = pa23

PA38_PREV_STAGE_pa1 =
PA38_PREV_STAGE_pa2 = pa1
PA38_PREV_STAGE_pa3 = pa2
PA38_PREV_STAGE_pa4 = pa3
PA38_PREV_STAGE_pa5 = pa4
PA38_PREV_STAGE_pa6 = pa5
PA38_PREV_STAGE_pa7 = pa6
PA38_PREV_STAGE_pa8 = pa7
PA38_PREV_STAGE_pa9 = pa8
PA38_PREV_STAGE_pa10 = pa9
PA38_PREV_STAGE_pa11 = pa10
PA38_PREV_STAGE_pa12 = pa11
PA38_PREV_STAGE_pa13 = pa12
PA38_PREV_STAGE_pa14 = pa13
PA38_PREV_STAGE_pa15 = pa14
PA38_PREV_STAGE_pa16 = pa15
PA38_PREV_STAGE_pa17 = pa16
PA38_PREV_STAGE_pa18 = pa17
PA38_PREV_STAGE_pa19 = pa18
PA38_PREV_STAGE_pa20 = pa19
PA38_PREV_STAGE_pa21 = pa20
PA38_PREV_STAGE_pa22 = pa21
PA38_PREV_STAGE_pa24 = pa22
PA38_PREV_STAGE_pa23 = pa24
PA38_PREV_STAGE_pa25 = pa23
PA38_PREV_STAGE_pa26 = pa25
PA38_PREV_STAGE_pa27 = pa26
PA38_PREV_STAGE_pa28 = pa27
PA38_PREV_STAGE_pa29 = pa28
PA38_PREV_STAGE_pa30 = pa29
PA38_PREV_STAGE_pa32 = pa30
PA38_PREV_STAGE_pa33 = pa32
PA38_PREV_STAGE_pa34 = pa33
PA38_PREV_STAGE_pa35 = pa34
PA38_PREV_STAGE_pa36 = pa35
PA38_PREV_STAGE_pa37 = pa36

PA38_STAGE_ARGS_pa1 =
PA38_STAGE_ARGS_pa2 =
PA38_STAGE_ARGS_pa3 =
PA38_STAGE_ARGS_pa4 =
PA38_STAGE_ARGS_pa5 =
PA38_STAGE_ARGS_pa6 =
PA38_STAGE_ARGS_pa7 =
PA38_STAGE_ARGS_pa8 =
PA38_STAGE_ARGS_pa9 =
PA38_STAGE_ARGS_pa10 = --emit-ast
PA38_STAGE_ARGS_pa11 = --emit-types
PA38_STAGE_ARGS_pa12 = --emit-semantics
PA38_STAGE_ARGS_pa13 =
PA38_STAGE_ARGS_pa14 = --emit-lowir -O0
PA38_STAGE_ARGS_pa15 = --emit-lowir -O0
PA38_STAGE_ARGS_pa16 = --emit-lowir -O0
PA38_STAGE_ARGS_pa17 = --emit-lowir -O0
PA38_STAGE_ARGS_pa18 = --emit-lowir -O0
PA38_STAGE_ARGS_pa19 = --emit-lowir -O0
PA38_STAGE_ARGS_pa20 = --emit-lowir -O0
PA38_STAGE_ARGS_pa21 = --emit-lowir -O0
PA38_STAGE_ARGS_pa22 = --emit-lowir -O0
PA38_STAGE_ARGS_pa24 = --emit-lowir -O0
PA38_STAGE_ARGS_pa23 =
PA38_STAGE_ARGS_pa25 =
PA38_STAGE_ARGS_pa26 = --emit-lowir -O0
PA38_STAGE_ARGS_pa27 = --emit-lowir -O0
PA38_STAGE_ARGS_pa28 = --emit-lowir -O0
PA38_STAGE_ARGS_pa29 = --emit-lowir -O0
PA38_STAGE_ARGS_pa30 =
PA38_STAGE_ARGS_pa32 =
PA38_STAGE_ARGS_pa33 =
PA38_STAGE_ARGS_pa34 =
PA38_STAGE_ARGS_pa35 =
PA38_STAGE_ARGS_pa36 =
PA38_STAGE_ARGS_pa37 =

PA38_COURSE_STAGE_pa1 = course/pa1
PA38_COURSE_STAGE_pa2 = course/pa2
PA38_COURSE_STAGE_pa3 = course/pa3
PA38_COURSE_STAGE_pa4 = course/pa4
PA38_COURSE_STAGE_pa5 = course/pa5
PA38_COURSE_STAGE_pa6 = course/pa6
PA38_COURSE_STAGE_pa7 = course/pa7
PA38_COURSE_STAGE_pa8 = course/pa8
PA38_COURSE_STAGE_pa9 = course/pa9
PA38_COURSE_STAGE_pa10 = course/pa10
PA38_COURSE_STAGE_pa11 = course/pa11
PA38_COURSE_STAGE_pa12 = course/pa12
PA38_COURSE_STAGE_pa13 = course/pa13
PA38_COURSE_STAGE_pa14 = course/pa14
PA38_COURSE_STAGE_pa15 = course/pa15
PA38_COURSE_STAGE_pa16 = course/pa16
PA38_COURSE_STAGE_pa17 = course/pa17
PA38_COURSE_STAGE_pa18 = course/pa18
PA38_COURSE_STAGE_pa19 = course/pa19
PA38_COURSE_STAGE_pa20 = course/pa20
PA38_COURSE_STAGE_pa21 = course/pa21
PA38_COURSE_STAGE_pa22 = course/pa22
PA38_COURSE_STAGE_pa24 = course/pa24
PA38_COURSE_STAGE_pa23 = course/pa23
PA38_COURSE_STAGE_pa25 = course/pa25
PA38_COURSE_STAGE_pa26 = course/pa26
PA38_COURSE_STAGE_pa27 = course/pa27
PA38_COURSE_STAGE_pa28 = course/pa28
PA38_COURSE_STAGE_pa29 = course/pa29
PA38_COURSE_STAGE_pa30 = course/pa30
PA38_COURSE_STAGE_pa32 = course/pa32
PA38_COURSE_STAGE_pa33 = course/pa33
PA38_COURSE_STAGE_pa34 = course/pa34
PA38_COURSE_STAGE_pa35 = course/pa35
PA38_COURSE_STAGE_pa36 = course/pa36
PA38_COURSE_STAGE_pa37 = course/pa37

pa38_checkpoint_entry = ../dev/$(1).cpp
$(foreach checkpoint,$(CHECKPOINTS),$(if $(strip $(FRONTEND_OBJ_BASENAMES_$(checkpoint))),,$(error missing FRONTEND_OBJ_BASENAMES_$(checkpoint) in ../dev/frontend_source_sets.mk)))
pa38_checkpoint_sources = $(addprefix ../dev/src/,$(addsuffix .cpp,$(FRONTEND_OBJ_BASENAMES_$(1))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval PA38_ENTRY_$(checkpoint) := $(call pa38_checkpoint_entry,$(checkpoint))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval PA38_SOURCES_$(checkpoint) := $(call pa38_checkpoint_sources,$(checkpoint))))

PA38_ALL_SHARED_SOURCES := $(sort $(foreach checkpoint,$(CHECKPOINTS),$(PA38_SOURCES_$(checkpoint))))

pa38_objdir = $(PA38_OBJ_ROOT)/$(1)/release
pa38_depdir = $(call pa38_objdir,$(1))/.d
PA38_SHARED_OBJDIR ?= $(PA38_OBJ_ROOT)/shared/release
PA38_SHARED_DEPDIR ?= $(PA38_SHARED_OBJDIR)/.d
pa38_stem = $(basename $(notdir $(1)))
pa38_entry_suffix = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-runner,-plain)
pa38_runner_suffix = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-enabled,-disabled)
pa38_binary_path = $(PA38_BIN_ROOT)/$(1)$(PA38_OUTPUT_SUFFIX)
pa38_link_output_path = $(PA38_BIN_ROOT)/$(1)
pa38_self_binary_path = $(PA38_BIN_ROOT_BASE)/selfhost/$(1)-self
pa38_inception_binary_path = $(PA38_BIN_ROOT_BASE)/inception/$(1)-inception
PA38_RESTORED_CPPGM_SELF ?= $(call pa38_self_binary_path,cppgm++)
pa38_entry_object_path = $(call pa38_objdir,$(1))/$(call pa38_stem,$(PA38_ENTRY_$(1)))$(pa38_entry_suffix).o
pa38_entry_depfile_path = $(call pa38_depdir,$(1))/$(call pa38_stem,$(PA38_ENTRY_$(1)))$(pa38_entry_suffix).d
pa38_runner_object_path = $(PA38_SHARED_OBJDIR)/test_runner$(pa38_runner_suffix).o
pa38_runner_depfile_path = $(PA38_SHARED_DEPDIR)/test_runner$(pa38_runner_suffix).d
pa38_shared_object_path = $(PA38_SHARED_OBJDIR)/$(call pa38_stem,$(1)).o
pa38_shared_depfile_path = $(PA38_SHARED_DEPDIR)/$(call pa38_stem,$(1)).d
pa38_object_list = $(foreach src,$(PA38_SOURCES_$(1)),$(call pa38_shared_object_path,$(src)))
pa38_link_inputs = $(call pa38_entry_object_path,$(1)) $(call pa38_object_list,$(1)) $(if $(filter 1,$(CPPGM_TEST_RUNNER)),$(pa38_runner_object_path))
pa38_stage_binary = $(call pa38_self_binary_path,$(PA38_CHECKPOINT_FOR_$(1)))
PA38_INCEPTION_CXX ?= $(call pa38_self_binary_path,cppgm++)

all: $(TARGETS)

test: $(PA_TEST_TARGETS)

ladder: $(TARGETS)

preservation: $(PA_TEST_TARGETS)

inception: $(INCEPTION_TARGETS)

compare-inception: $(INCEPTION_COMPARE_TARGETS)

bitcmp: compare-inception

cppgm++-inception-from-restored-self:
	@if [ ! -x "$(PA38_RESTORED_CPPGM_SELF)" ]; then \
		echo "missing restored self compiler: $(PA38_RESTORED_CPPGM_SELF)" >&2; \
		exit 2; \
	fi
	$(MAKE) "$(call pa38_inception_binary_path,cppgm++)" \
		PA38_COMPILER_FLAVOR=inception \
		PA38_OUTPUT_SUFFIX=-inception \
		PA38_OBJ_ROOT_BASE="$(PA38_OBJ_ROOT_BASE)" \
		PA38_BIN_ROOT_BASE="$(PA38_BIN_ROOT_BASE)" \
		PA38_SELFHOST_OPT_LEVEL="$(PA38_SELFHOST_OPT_LEVEL)" \
		CXX="$(PA38_RESTORED_CPPGM_SELF)" \
		CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
		CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
		PA38_DEFINES='$(PA38_DEFINES)'
	@ln -sf $(call pa38_inception_binary_path,cppgm++) cppgm++-inception

compare-cppgm++-inception-from-restored-self: cppgm++-inception-from-restored-self
	@cmp -s "$(PA38_RESTORED_CPPGM_SELF)" "$(call pa38_inception_binary_path,cppgm++)" || \
		(echo "mismatch: $(PA38_RESTORED_CPPGM_SELF) != $(call pa38_inception_binary_path,cppgm++)" >&2; exit 1)
	@echo "MATCH cppgm++: $(call pa38_inception_binary_path,cppgm++)"

compiler-ready:
	@if [ -x "$(CXX)" ]; then \
		exit 0; \
	fi; \
	if command -v "$(CXX)" >/dev/null 2>&1; then \
		exit 0; \
	fi; \
	if [ ! -x "$(CXX)" ]; then \
		if [ "$(CXX)" = "../dev/cppgm++" ]; then \
			$(MAKE) -C ../dev cppgm++ \
				CXX="$(CPPGM_HOST_CXX)" \
				CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
				CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)"; \
		else \
			echo "missing compiler: $(CXX)" >&2; \
			exit 1; \
		fi; \
	fi

$(PA38_BUILTIN_HOST_CONFIG): FORCE
	@mkdir -p $(dir $@)
	@CPPGM_BUILD_HOST_CXX='$(CPPGM_HOST_CXX)' \
		CPPGM_STDLIB_FLAGS='$(CPPGM_STDLIB_FLAGS)' \
		perl ../dev/gen_builtin_host_config.pl > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv $@.tmp $@; else rm -f $@.tmp; fi

../dev/cppgm++:
	$(MAKE) -C ../dev cppgm++ \
		CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)"

define PA38_SOURCE_RULE
$(call pa38_shared_object_path,$(1)): $(1) $(PA38_CXX_DEP) | compiler-ready $(PA38_SHARED_OBJDIR) $(PA38_SHARED_DEPDIR)
	@mkdir -p $$(@D) $(PA38_SHARED_DEPDIR)
	$(CXX) $(PA38_CC_FLAGS) $(foreach inc,$(PA38_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA38_DEFINES),-D$(def)) -MMD -MP -MF $(call pa38_shared_depfile_path,$(1)) -c -o $$@ $$<
endef

$(PA38_BIN_ROOT):
	@mkdir -p $@

$(PA38_SHARED_OBJDIR) $(PA38_SHARED_DEPDIR):
	@mkdir -p $@

$(call pa38_shared_object_path,../dev/src/preprocessor.cpp): $(PA38_BUILTIN_HOST_CONFIG)

$(pa38_runner_object_path): ../dev/src/test_runner.cpp $(PA38_CXX_DEP) | compiler-ready $(PA38_SHARED_OBJDIR) $(PA38_SHARED_DEPDIR)
	@mkdir -p $(@D) $(PA38_SHARED_DEPDIR)
	$(CXX) $(PA38_CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(foreach inc,$(PA38_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA38_DEFINES),-D$(def)) -MMD -MP -MF $(pa38_runner_depfile_path) -c -o $@ $<

define PA38_CHECKPOINT_RULES
$(call pa38_objdir,$(1)) $(call pa38_depdir,$(1)):
	@mkdir -p $$@

$(call pa38_entry_object_path,$(1)): $(PA38_ENTRY_$(1)) $(PA38_CXX_DEP) | compiler-ready $(call pa38_objdir,$(1)) $(call pa38_depdir,$(1))
	@mkdir -p $$(@D) $(call pa38_depdir,$(1))
	$(CXX) $(PA38_ENTRY_CC_FLAGS) $(foreach inc,$(PA38_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA38_DEFINES),-D$(def)) -MMD -MP -MF $(call pa38_entry_depfile_path,$(1)) -c -o $$@ $$<

$(call pa38_binary_path,$(1)): $(call pa38_link_inputs,$(1)) | $(PA38_BIN_ROOT)
	@rm -f "$(call pa38_link_output_path,$(1))"
	$(CPPGM_HOST_CXX) $(PA38_LINK_FLAGS) -I../dev/src -o "$(call pa38_link_output_path,$(1))" $$^
	@mv -f "$(call pa38_link_output_path,$(1))" $$@

$(1)-self: $(call pa38_binary_path,$(1))
	@ln -sf $(call pa38_binary_path,$(1)) $$@

test-$(1): $(1)-self
	$$(MAKE) test-$(PA38_PRIMARY_STAGE_$(1))-nobuild

through-$(1): $(if $(PA38_PREV_$(1)),through-$(PA38_PREV_$(1)))
	$$(MAKE) $(1)-self

test-through-$(1): $(if $(PA38_PREV_$(1)),test-through-$(PA38_PREV_$(1)))
	$$(MAKE) test-$(1)
endef

define PA38_STAGE_RULES
test-$(1): $(PA38_CHECKPOINT_FOR_$(1))-self
	$$(MAKE) test-$(1)-nobuild

test-$(1)-nobuild:
	@if [ ! -x "$(call pa38_stage_binary,$(1))" ]; then \
		echo "missing staged binary: $(call pa38_stage_binary,$(1))" >&2; \
		echo "build it first with 'make through-$(1)' or 'make $(PA38_CHECKPOINT_FOR_$(1))-self'" >&2; \
		exit 2; \
	fi
	@if [ "$(1)" = "pa36" ]; then \
		$(call pa38_stage_test_env,$(1)) $(MAKE) -C ../pa36 \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			LOWIROPT_APP="$(call pa38_stage_binary,$(1))" \
			CPPGM_DRIVER_APP="$(call pa38_binary_path,cppgm++)" \
			test; \
	elif [ "$(1)" = "pa37" ]; then \
		$(call pa38_stage_test_env,$(1)) $(MAKE) -C ../pa37 \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			LOWIR2NATIVE_APP="$(call pa38_stage_binary,$(1))" \
			test; \
	else \
		$(call pa38_stage_test_env,$(1)) $(MAKE) -C ../$(1) \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			CPPGM_TEST_APP="$(call pa38_stage_binary,$(1))" \
			CPPGM_APP_ARGS='$(PA38_STAGE_ARGS_$(1))' \
			test; \
	fi

through-$(1): $(if $(PA38_PREV_STAGE_$(1)),through-$(PA38_PREV_STAGE_$(1)))
	$$(MAKE) $(PA38_CHECKPOINT_FOR_$(1))-self

test-through-$(1): $(if $(PA38_PREV_STAGE_$(1)),test-through-$(PA38_PREV_STAGE_$(1)))
	$$(MAKE) test-$(1)
endef

define PA38_INCEPTION_RULES
$(1)-inception: cppgm++-self
	$$(MAKE) "$(call pa38_inception_binary_path,$(1))" \
		PA38_COMPILER_FLAVOR=inception \
		PA38_OUTPUT_SUFFIX=-inception \
		PA38_OBJ_ROOT_BASE="$(PA38_OBJ_ROOT_BASE)" \
		PA38_BIN_ROOT_BASE="$(PA38_BIN_ROOT_BASE)" \
		PA38_SELFHOST_OPT_LEVEL="$(PA38_SELFHOST_OPT_LEVEL)" \
		CXX="$(PA38_INCEPTION_CXX)" \
		CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
		CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
		PA38_DEFINES='$(PA38_DEFINES)'
	@ln -sf $(call pa38_inception_binary_path,$(1)) $$@

compare-$(1)-inception: $(1)-self $(1)-inception
	@cmp -s "$(call pa38_self_binary_path,$(1))" "$(call pa38_inception_binary_path,$(1))" || \
		(echo "mismatch: $(call pa38_self_binary_path,$(1)) != $(call pa38_inception_binary_path,$(1))" >&2; exit 1)
	@echo "MATCH $(1): $(call pa38_inception_binary_path,$(1))"
endef

$(foreach checkpoint,$(CHECKPOINTS),$(eval $(call PA38_CHECKPOINT_RULES,$(checkpoint))))
$(foreach src,$(PA38_ALL_SHARED_SOURCES),$(eval $(call PA38_SOURCE_RULE,$(src))))
$(foreach pa,$(PA_STAGE_NAMES),$(eval $(call PA38_STAGE_RULES,$(pa))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval $(call PA38_INCEPTION_RULES,$(checkpoint))))

test-pa36: cppgm++-self

clean:
	-rm -f $(TARGETS) $(INCEPTION_TARGETS)
	-rm -rf $(PA38_OBJ_ROOT_BASE)

.PHONY: all test ladder preservation clean compiler-ready \
	inception compare-inception bitcmp \
	cppgm++-inception-from-restored-self \
	compare-cppgm++-inception-from-restored-self \
	$(TARGETS) $(INCEPTION_TARGETS) $(INCEPTION_COMPARE_TARGETS) \
	$(TEST_TARGETS) $(THROUGH_TARGETS) $(TEST_THROUGH_TARGETS) \
	$(PA_THROUGH_TARGETS) $(PA_TEST_THROUGH_TARGETS) $(PA_TEST_TARGETS) \
	$(PA_TEST_NOBUILD_TARGETS)

-include $(foreach checkpoint,$(CHECKPOINTS),$(call pa38_entry_depfile_path,$(checkpoint)))
-include $(foreach src,$(PA38_ALL_SHARED_SOURCES),$(call pa38_shared_depfile_path,$(src)))
-include $(pa38_runner_depfile_path)

FORCE:

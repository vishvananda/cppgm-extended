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
	cpplink-self \
	cppeh-self \
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
export CPPGM_TEST_JOBS
export CPPGM_HOST_CXX
export CPPGM_STDLIB_FLAGS
PA37_SELFHOST_OPT_LEVEL ?= 3
PA37_OBJ_ROOT_BASE ?= ../obj/pa37
PA37_BIN_ROOT_BASE ?= $(PA37_OBJ_ROOT_BASE)/bin
PA37_GENERATED ?= $(PA37_OBJ_ROOT_BASE)/generated
PA37_BUILTIN_HOST_CONFIG = $(PA37_GENERATED)/cppgm_builtin_host_config.h
PA37_INCLUDE_PATHS := ../dev/src $(PA37_GENERATED)
PA37_DEFINES ?=
HOST_CXX_DEFAULT_DEF = -DCPPGM_DEFAULT_HOST_CXX='"$(CPPGM_HOST_CXX)"'
OBJECT_ROOT_DEFAULT_DEF = -DCPPGM_DEFAULT_OBJECT_ROOT='"$(abspath $(PA37_OBJ_ROOT_BASE))"'
PA37_CC_FLAGS = -std=gnu++11 -Wall -O$(PA37_SELFHOST_OPT_LEVEL) $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)
PA37_ENTRY_CC_FLAGS = $(PA37_CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
PA37_LINKER_DETERMINISM_FLAGS =
ifeq ($(HOST_UNAME_S),Darwin)
PA37_LINKER_DETERMINISM_FLAGS += -Wl,-reproducible
endif
PA37_LINK_FLAGS = -std=gnu++11 $(CPPGM_STDLIB_FLAGS) $(PA37_LINKER_DETERMINISM_FLAGS)
include ../dev/frontend_source_sets.mk

ifeq ($(abspath $(CXX)),$(abspath ../dev/cppgm++))
PA37_COMPILER_FLAVOR ?= selfhost
else
PA37_COMPILER_FLAVOR ?= host
endif

ifneq ($(filter ./% ../% /%,$(CXX)),)
PA37_CXX_DEP := $(CXX)
else
PA37_CXX_DEP :=
endif

PA37_OBJ_ROOT ?= $(PA37_OBJ_ROOT_BASE)/$(PA37_COMPILER_FLAVOR)
PA37_BIN_ROOT ?= $(PA37_BIN_ROOT_BASE)/$(PA37_COMPILER_FLAVOR)
PA37_OUTPUT_SUFFIX ?= -self

CHECKPOINTS = pptoken posttoken ctrlexpr macro preproc recog nsdecl nsinit cy86 cppgm++ lowiropt lowir2cy86 lowir2native cpplink cppeh
TEST_TARGETS = $(addprefix test-,$(CHECKPOINTS))
THROUGH_TARGETS = $(addprefix through-,$(CHECKPOINTS))
TEST_THROUGH_TARGETS = $(addprefix test-through-,$(CHECKPOINTS))
INCEPTION_TARGETS = $(addsuffix -inception,$(CHECKPOINTS))
INCEPTION_COMPARE_TARGETS = $(addprefix compare-,$(INCEPTION_TARGETS))
PA_STAGE_NAMES = pa1 pa2 pa3 pa4 pa5 pa6 pa7 pa8 pa9 pa10 pa11 pa12 pa13 pa14 pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22 pa23 pa24 pa25 pa26 pa27 pa28 pa29 pa30 pa31 pa32 pa33 pa34 pa35 pa36
PA37_BATCH_TEST_STAGE_NAMES = pa10 pa11 pa12 pa13 pa14 pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22 pa23 pa24 pa25 pa26 pa27 pa28 pa29 pa30 pa31 pa32 pa33 pa34 pa35 pa36
PA37_BATCH_TEST_ENV = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),CPPGM_BATCH_TESTS=1 WRAPPED_BATCH_STDIN=1,)
pa37_stage_test_env = $(if $(filter $(1),$(PA37_BATCH_TEST_STAGE_NAMES)),$(PA37_BATCH_TEST_ENV),)
PA_THROUGH_TARGETS = $(addprefix through-,$(PA_STAGE_NAMES))
PA_TEST_THROUGH_TARGETS = $(addprefix test-through-,$(PA_STAGE_NAMES))
PA_TEST_TARGETS = $(addprefix test-,$(PA_STAGE_NAMES))
PA_TEST_NOBUILD_TARGETS = $(addsuffix -nobuild,$(PA_TEST_TARGETS))

PA37_PREV_pptoken =
PA37_PREV_posttoken = pptoken
PA37_PREV_ctrlexpr = posttoken
PA37_PREV_macro = ctrlexpr
PA37_PREV_preproc = macro
PA37_PREV_recog = preproc
PA37_PREV_nsdecl = recog
PA37_PREV_nsinit = nsdecl
PA37_PREV_cy86 = nsinit
PA37_PREV_cppgm++ = cy86
PA37_PREV_lowiropt = cppgm++
PA37_PREV_lowir2cy86 = cppgm++
PA37_PREV_lowir2native = lowir2cy86
PA37_PREV_cpplink = lowir2native
PA37_PREV_cppeh = cpplink

PA37_CHECKPOINT_FOR_pa1 = pptoken
PA37_CHECKPOINT_FOR_pa2 = posttoken
PA37_CHECKPOINT_FOR_pa3 = ctrlexpr
PA37_CHECKPOINT_FOR_pa4 = macro
PA37_CHECKPOINT_FOR_pa5 = preproc
PA37_CHECKPOINT_FOR_pa6 = recog
PA37_CHECKPOINT_FOR_pa7 = nsdecl
PA37_CHECKPOINT_FOR_pa8 = nsinit
PA37_CHECKPOINT_FOR_pa9 = cy86
PA37_CHECKPOINT_FOR_pa10 = cppgm++
PA37_CHECKPOINT_FOR_pa11 = cppgm++
PA37_CHECKPOINT_FOR_pa12 = cppgm++
PA37_CHECKPOINT_FOR_pa13 = lowir2cy86
PA37_CHECKPOINT_FOR_pa14 = cppgm++
PA37_CHECKPOINT_FOR_pa15 = cppgm++
PA37_CHECKPOINT_FOR_pa16 = cppgm++
PA37_CHECKPOINT_FOR_pa17 = cppgm++
PA37_CHECKPOINT_FOR_pa18 = cppgm++
PA37_CHECKPOINT_FOR_pa19 = cppgm++
PA37_CHECKPOINT_FOR_pa20 = cppgm++
PA37_CHECKPOINT_FOR_pa21 = cppgm++
PA37_CHECKPOINT_FOR_pa22 = cppgm++
PA37_CHECKPOINT_FOR_pa23 = lowir2native
PA37_CHECKPOINT_FOR_pa24 = cpplink
PA37_CHECKPOINT_FOR_pa25 = cppeh
PA37_CHECKPOINT_FOR_pa26 = cppgm++
PA37_CHECKPOINT_FOR_pa27 = cppgm++
PA37_CHECKPOINT_FOR_pa28 = cppgm++
PA37_CHECKPOINT_FOR_pa29 = cppgm++
PA37_CHECKPOINT_FOR_pa30 = cppgm++
PA37_CHECKPOINT_FOR_pa31 = cppgm++
PA37_CHECKPOINT_FOR_pa32 = cppgm++
PA37_CHECKPOINT_FOR_pa33 = cppgm++
PA37_CHECKPOINT_FOR_pa34 = cppgm++
PA37_CHECKPOINT_FOR_pa35 = lowiropt
PA37_CHECKPOINT_FOR_pa36 = lowir2native
PA37_PRIMARY_STAGE_pptoken = pa1
PA37_PRIMARY_STAGE_posttoken = pa2
PA37_PRIMARY_STAGE_ctrlexpr = pa3
PA37_PRIMARY_STAGE_macro = pa4
PA37_PRIMARY_STAGE_preproc = pa5
PA37_PRIMARY_STAGE_recog = pa6
PA37_PRIMARY_STAGE_nsdecl = pa7
PA37_PRIMARY_STAGE_nsinit = pa8
PA37_PRIMARY_STAGE_cy86 = pa9
PA37_PRIMARY_STAGE_cppgm++ = pa10
PA37_PRIMARY_STAGE_lowiropt = pa35
PA37_PRIMARY_STAGE_lowir2cy86 = pa13
PA37_PRIMARY_STAGE_lowir2native = pa23
PA37_PRIMARY_STAGE_cpplink = pa24
PA37_PRIMARY_STAGE_cppeh = pa25

PA37_PREV_STAGE_pa1 =
PA37_PREV_STAGE_pa2 = pa1
PA37_PREV_STAGE_pa3 = pa2
PA37_PREV_STAGE_pa4 = pa3
PA37_PREV_STAGE_pa5 = pa4
PA37_PREV_STAGE_pa6 = pa5
PA37_PREV_STAGE_pa7 = pa6
PA37_PREV_STAGE_pa8 = pa7
PA37_PREV_STAGE_pa9 = pa8
PA37_PREV_STAGE_pa10 = pa9
PA37_PREV_STAGE_pa11 = pa10
PA37_PREV_STAGE_pa12 = pa11
PA37_PREV_STAGE_pa13 = pa12
PA37_PREV_STAGE_pa14 = pa13
PA37_PREV_STAGE_pa15 = pa14
PA37_PREV_STAGE_pa16 = pa15
PA37_PREV_STAGE_pa17 = pa16
PA37_PREV_STAGE_pa18 = pa17
PA37_PREV_STAGE_pa19 = pa18
PA37_PREV_STAGE_pa20 = pa19
PA37_PREV_STAGE_pa21 = pa20
PA37_PREV_STAGE_pa22 = pa21
PA37_PREV_STAGE_pa23 = pa22
PA37_PREV_STAGE_pa24 = pa23
PA37_PREV_STAGE_pa25 = pa24
PA37_PREV_STAGE_pa26 = pa25
PA37_PREV_STAGE_pa27 = pa26
PA37_PREV_STAGE_pa28 = pa27
PA37_PREV_STAGE_pa29 = pa28
PA37_PREV_STAGE_pa30 = pa29
PA37_PREV_STAGE_pa31 = pa30
PA37_PREV_STAGE_pa32 = pa31
PA37_PREV_STAGE_pa33 = pa32
PA37_PREV_STAGE_pa34 = pa33
PA37_PREV_STAGE_pa35 = pa34
PA37_PREV_STAGE_pa36 = pa35

PA37_STAGE_ARGS_pa1 =
PA37_STAGE_ARGS_pa2 =
PA37_STAGE_ARGS_pa3 =
PA37_STAGE_ARGS_pa4 =
PA37_STAGE_ARGS_pa5 =
PA37_STAGE_ARGS_pa6 =
PA37_STAGE_ARGS_pa7 =
PA37_STAGE_ARGS_pa8 =
PA37_STAGE_ARGS_pa9 =
PA37_STAGE_ARGS_pa10 = --emit-ast
PA37_STAGE_ARGS_pa11 = --emit-types
PA37_STAGE_ARGS_pa12 = --emit-semantics
PA37_STAGE_ARGS_pa13 =
PA37_STAGE_ARGS_pa14 = --emit-lowir -O0
PA37_STAGE_ARGS_pa15 = --emit-lowir -O0
PA37_STAGE_ARGS_pa16 = --emit-lowir -O0
PA37_STAGE_ARGS_pa17 = --emit-lowir -O0
PA37_STAGE_ARGS_pa18 = --emit-lowir -O0
PA37_STAGE_ARGS_pa19 = --emit-lowir -O0
PA37_STAGE_ARGS_pa20 = --emit-lowir -O0
PA37_STAGE_ARGS_pa21 = --emit-lowir -O0
PA37_STAGE_ARGS_pa22 = --emit-lowir -O0
PA37_STAGE_ARGS_pa23 =
PA37_STAGE_ARGS_pa24 =
PA37_STAGE_ARGS_pa25 =
PA37_STAGE_ARGS_pa26 = --emit-lowir -O0
PA37_STAGE_ARGS_pa27 = --emit-lowir -O0
PA37_STAGE_ARGS_pa28 = --emit-lowir -O0
PA37_STAGE_ARGS_pa29 = --emit-lowir -O0
PA37_STAGE_ARGS_pa30 =
PA37_STAGE_ARGS_pa31 =
PA37_STAGE_ARGS_pa32 =
PA37_STAGE_ARGS_pa33 =
PA37_STAGE_ARGS_pa34 =
PA37_STAGE_ARGS_pa35 =
PA37_STAGE_ARGS_pa36 =

PA37_COURSE_STAGE_pa1 = course/pa1
PA37_COURSE_STAGE_pa2 = course/pa2
PA37_COURSE_STAGE_pa3 = course/pa3
PA37_COURSE_STAGE_pa4 = course/pa4
PA37_COURSE_STAGE_pa5 = course/pa5
PA37_COURSE_STAGE_pa6 = course/pa6
PA37_COURSE_STAGE_pa7 = course/pa7
PA37_COURSE_STAGE_pa8 = course/pa8
PA37_COURSE_STAGE_pa9 = course/pa9
PA37_COURSE_STAGE_pa10 = course/pa10
PA37_COURSE_STAGE_pa11 = course/pa11
PA37_COURSE_STAGE_pa12 = course/pa12
PA37_COURSE_STAGE_pa13 = course/pa13
PA37_COURSE_STAGE_pa14 = course/pa14
PA37_COURSE_STAGE_pa15 = course/pa15
PA37_COURSE_STAGE_pa16 = course/pa16
PA37_COURSE_STAGE_pa17 = course/pa17
PA37_COURSE_STAGE_pa18 = course/pa18
PA37_COURSE_STAGE_pa19 = course/pa19
PA37_COURSE_STAGE_pa20 = course/pa20
PA37_COURSE_STAGE_pa21 = course/pa21
PA37_COURSE_STAGE_pa22 = course/pa22
PA37_COURSE_STAGE_pa23 = course/pa23
PA37_COURSE_STAGE_pa24 = course/pa24
PA37_COURSE_STAGE_pa25 = course/pa25
PA37_COURSE_STAGE_pa26 = course/pa26
PA37_COURSE_STAGE_pa27 = course/pa27
PA37_COURSE_STAGE_pa28 = course/pa28
PA37_COURSE_STAGE_pa29 = course/pa29
PA37_COURSE_STAGE_pa30 = course/pa30
PA37_COURSE_STAGE_pa31 = course/pa31
PA37_COURSE_STAGE_pa32 = course/pa32
PA37_COURSE_STAGE_pa33 = course/pa33
PA37_COURSE_STAGE_pa34 = course/pa34
PA37_COURSE_STAGE_pa35 = course/pa35
PA37_COURSE_STAGE_pa36 = course/pa36

pa37_checkpoint_entry = ../dev/$(1).cpp
$(foreach checkpoint,$(CHECKPOINTS),$(if $(strip $(FRONTEND_OBJ_BASENAMES_$(checkpoint))),,$(error missing FRONTEND_OBJ_BASENAMES_$(checkpoint) in ../dev/frontend_source_sets.mk)))
pa37_checkpoint_sources = $(addprefix ../dev/src/,$(addsuffix .cpp,$(FRONTEND_OBJ_BASENAMES_$(1))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval PA37_ENTRY_$(checkpoint) := $(call pa37_checkpoint_entry,$(checkpoint))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval PA37_SOURCES_$(checkpoint) := $(call pa37_checkpoint_sources,$(checkpoint))))

PA37_ALL_SHARED_SOURCES := $(sort $(foreach checkpoint,$(CHECKPOINTS),$(PA37_SOURCES_$(checkpoint))))

pa37_objdir = $(PA37_OBJ_ROOT)/$(1)/release
pa37_depdir = $(call pa37_objdir,$(1))/.d
PA37_SHARED_OBJDIR ?= $(PA37_OBJ_ROOT)/shared/release
PA37_SHARED_DEPDIR ?= $(PA37_SHARED_OBJDIR)/.d
pa37_stem = $(basename $(notdir $(1)))
pa37_entry_suffix = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-runner,-plain)
pa37_runner_suffix = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-enabled,-disabled)
pa37_binary_path = $(PA37_BIN_ROOT)/$(1)$(PA37_OUTPUT_SUFFIX)
pa37_link_output_path = $(PA37_BIN_ROOT)/$(1)
pa37_self_binary_path = $(PA37_BIN_ROOT_BASE)/selfhost/$(1)-self
pa37_inception_binary_path = $(PA37_BIN_ROOT_BASE)/inception/$(1)-inception
PA37_RESTORED_CPPGM_SELF ?= $(call pa37_self_binary_path,cppgm++)
pa37_entry_object_path = $(call pa37_objdir,$(1))/$(call pa37_stem,$(PA37_ENTRY_$(1)))$(pa37_entry_suffix).o
pa37_entry_depfile_path = $(call pa37_depdir,$(1))/$(call pa37_stem,$(PA37_ENTRY_$(1)))$(pa37_entry_suffix).d
pa37_runner_object_path = $(PA37_SHARED_OBJDIR)/test_runner$(pa37_runner_suffix).o
pa37_runner_depfile_path = $(PA37_SHARED_DEPDIR)/test_runner$(pa37_runner_suffix).d
pa37_shared_object_path = $(PA37_SHARED_OBJDIR)/$(call pa37_stem,$(1)).o
pa37_shared_depfile_path = $(PA37_SHARED_DEPDIR)/$(call pa37_stem,$(1)).d
pa37_object_list = $(foreach src,$(PA37_SOURCES_$(1)),$(call pa37_shared_object_path,$(src)))
pa37_link_inputs = $(call pa37_entry_object_path,$(1)) $(call pa37_object_list,$(1)) $(if $(filter 1,$(CPPGM_TEST_RUNNER)),$(pa37_runner_object_path))
pa37_stage_binary = $(call pa37_self_binary_path,$(PA37_CHECKPOINT_FOR_$(1)))
PA37_INCEPTION_CXX ?= $(call pa37_self_binary_path,cppgm++)

all: $(TARGETS)

test: $(PA_TEST_TARGETS)

ladder: $(TARGETS)

preservation: $(PA_TEST_TARGETS)

inception: $(INCEPTION_TARGETS)

compare-inception: $(INCEPTION_COMPARE_TARGETS)

bitcmp: compare-inception

cppgm++-inception-from-restored-self:
	@if [ ! -x "$(PA37_RESTORED_CPPGM_SELF)" ]; then \
		echo "missing restored self compiler: $(PA37_RESTORED_CPPGM_SELF)" >&2; \
		exit 2; \
	fi
	$(MAKE) "$(call pa37_inception_binary_path,cppgm++)" \
		PA37_COMPILER_FLAVOR=inception \
		PA37_OUTPUT_SUFFIX=-inception \
		PA37_OBJ_ROOT_BASE="$(PA37_OBJ_ROOT_BASE)" \
		PA37_BIN_ROOT_BASE="$(PA37_BIN_ROOT_BASE)" \
		PA37_SELFHOST_OPT_LEVEL="$(PA37_SELFHOST_OPT_LEVEL)" \
		CXX="$(PA37_RESTORED_CPPGM_SELF)" \
		CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
		CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
		PA37_DEFINES='$(PA37_DEFINES)'
	@ln -sf $(call pa37_inception_binary_path,cppgm++) cppgm++-inception

compare-cppgm++-inception-from-restored-self: cppgm++-inception-from-restored-self
	@cmp -s "$(PA37_RESTORED_CPPGM_SELF)" "$(call pa37_inception_binary_path,cppgm++)" || \
		(echo "mismatch: $(PA37_RESTORED_CPPGM_SELF) != $(call pa37_inception_binary_path,cppgm++)" >&2; exit 1)
	@echo "MATCH cppgm++: $(call pa37_inception_binary_path,cppgm++)"

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

$(PA37_BUILTIN_HOST_CONFIG): FORCE
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

define PA37_SOURCE_RULE
$(call pa37_shared_object_path,$(1)): $(1) $(PA37_CXX_DEP) | compiler-ready $(PA37_SHARED_OBJDIR) $(PA37_SHARED_DEPDIR)
	@mkdir -p $$(@D) $(PA37_SHARED_DEPDIR)
	$(CXX) $(PA37_CC_FLAGS) $(foreach inc,$(PA37_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA37_DEFINES),-D$(def)) -MMD -MP -MF $(call pa37_shared_depfile_path,$(1)) -c -o $$@ $$<
endef

$(PA37_BIN_ROOT):
	@mkdir -p $@

$(PA37_SHARED_OBJDIR) $(PA37_SHARED_DEPDIR):
	@mkdir -p $@

$(call pa37_shared_object_path,../dev/src/preprocessor.cpp): $(PA37_BUILTIN_HOST_CONFIG)

$(pa37_runner_object_path): ../dev/src/test_runner.cpp $(PA37_CXX_DEP) | compiler-ready $(PA37_SHARED_OBJDIR) $(PA37_SHARED_DEPDIR)
	@mkdir -p $(@D) $(PA37_SHARED_DEPDIR)
	$(CXX) $(PA37_CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(foreach inc,$(PA37_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA37_DEFINES),-D$(def)) -MMD -MP -MF $(pa37_runner_depfile_path) -c -o $@ $<

define PA37_CHECKPOINT_RULES
$(call pa37_objdir,$(1)) $(call pa37_depdir,$(1)):
	@mkdir -p $$@

$(call pa37_entry_object_path,$(1)): $(PA37_ENTRY_$(1)) $(PA37_CXX_DEP) | compiler-ready $(call pa37_objdir,$(1)) $(call pa37_depdir,$(1))
	@mkdir -p $$(@D) $(call pa37_depdir,$(1))
	$(CXX) $(PA37_ENTRY_CC_FLAGS) $(foreach inc,$(PA37_INCLUDE_PATHS),-I$(inc)) $(foreach def,$(PA37_DEFINES),-D$(def)) -MMD -MP -MF $(call pa37_entry_depfile_path,$(1)) -c -o $$@ $$<

$(call pa37_binary_path,$(1)): $(call pa37_link_inputs,$(1)) | $(PA37_BIN_ROOT)
	@rm -f "$(call pa37_link_output_path,$(1))"
	$(CPPGM_HOST_CXX) $(PA37_LINK_FLAGS) -I../dev/src -o "$(call pa37_link_output_path,$(1))" $$^
	@mv -f "$(call pa37_link_output_path,$(1))" $$@

$(1)-self: $(call pa37_binary_path,$(1))
	@ln -sf $(call pa37_binary_path,$(1)) $$@

test-$(1): $(1)-self
	$$(MAKE) test-$(PA37_PRIMARY_STAGE_$(1))-nobuild

through-$(1): $(if $(PA37_PREV_$(1)),through-$(PA37_PREV_$(1)))
	$$(MAKE) $(1)-self

test-through-$(1): $(if $(PA37_PREV_$(1)),test-through-$(PA37_PREV_$(1)))
	$$(MAKE) test-$(1)
endef

define PA37_STAGE_RULES
test-$(1): $(PA37_CHECKPOINT_FOR_$(1))-self
	$$(MAKE) test-$(1)-nobuild

test-$(1)-nobuild:
	@if [ ! -x "$(call pa37_stage_binary,$(1))" ]; then \
		echo "missing staged binary: $(call pa37_stage_binary,$(1))" >&2; \
		echo "build it first with 'make through-$(1)' or 'make $(PA37_CHECKPOINT_FOR_$(1))-self'" >&2; \
		exit 2; \
	fi
	@if [ "$(1)" = "pa35" ]; then \
		$(call pa37_stage_test_env,$(1)) $(MAKE) -C ../pa35 \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			LOWIROPT_APP="$(call pa37_stage_binary,$(1))" \
			CPPGM_DRIVER_APP="$(call pa37_binary_path,cppgm++)" \
			test; \
	elif [ "$(1)" = "pa36" ]; then \
		$(call pa37_stage_test_env,$(1)) $(MAKE) -C ../pa36 \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			LOWIR2NATIVE_APP="$(call pa37_stage_binary,$(1))" \
			test; \
	else \
		$(call pa37_stage_test_env,$(1)) $(MAKE) -C ../$(1) \
			CPPGM_SKIP_DEV_REBUILD=1 \
			TEST_DEPS= \
			CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
			CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
			CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
			CPPGM_TEST_APP="$(call pa37_stage_binary,$(1))" \
			CPPGM_APP_ARGS='$(PA37_STAGE_ARGS_$(1))' \
			test; \
	fi

through-$(1): $(if $(PA37_PREV_STAGE_$(1)),through-$(PA37_PREV_STAGE_$(1)))
	$$(MAKE) $(PA37_CHECKPOINT_FOR_$(1))-self

test-through-$(1): $(if $(PA37_PREV_STAGE_$(1)),test-through-$(PA37_PREV_STAGE_$(1)))
	$$(MAKE) test-$(1)
endef

define PA37_INCEPTION_RULES
$(1)-inception: cppgm++-self
	$$(MAKE) "$(call pa37_inception_binary_path,$(1))" \
		PA37_COMPILER_FLAVOR=inception \
		PA37_OUTPUT_SUFFIX=-inception \
		PA37_OBJ_ROOT_BASE="$(PA37_OBJ_ROOT_BASE)" \
		PA37_BIN_ROOT_BASE="$(PA37_BIN_ROOT_BASE)" \
		PA37_SELFHOST_OPT_LEVEL="$(PA37_SELFHOST_OPT_LEVEL)" \
		CXX="$(PA37_INCEPTION_CXX)" \
		CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" \
		CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" \
		CPPGM_TEST_RUNNER="$(CPPGM_TEST_RUNNER)" \
		PA37_DEFINES='$(PA37_DEFINES)'
	@ln -sf $(call pa37_inception_binary_path,$(1)) $$@

compare-$(1)-inception: $(1)-self $(1)-inception
	@cmp -s "$(call pa37_self_binary_path,$(1))" "$(call pa37_inception_binary_path,$(1))" || \
		(echo "mismatch: $(call pa37_self_binary_path,$(1)) != $(call pa37_inception_binary_path,$(1))" >&2; exit 1)
	@echo "MATCH $(1): $(call pa37_inception_binary_path,$(1))"
endef

$(foreach checkpoint,$(CHECKPOINTS),$(eval $(call PA37_CHECKPOINT_RULES,$(checkpoint))))
$(foreach src,$(PA37_ALL_SHARED_SOURCES),$(eval $(call PA37_SOURCE_RULE,$(src))))
$(foreach pa,$(PA_STAGE_NAMES),$(eval $(call PA37_STAGE_RULES,$(pa))))
$(foreach checkpoint,$(CHECKPOINTS),$(eval $(call PA37_INCEPTION_RULES,$(checkpoint))))

test-pa35: cppgm++-self

clean:
	-rm -f $(TARGETS) $(INCEPTION_TARGETS)
	-rm -rf $(PA37_OBJ_ROOT_BASE)

.PHONY: all test ladder preservation clean compiler-ready \
	inception compare-inception bitcmp \
	cppgm++-inception-from-restored-self \
	compare-cppgm++-inception-from-restored-self \
	$(TARGETS) $(INCEPTION_TARGETS) $(INCEPTION_COMPARE_TARGETS) \
	$(TEST_TARGETS) $(THROUGH_TARGETS) $(TEST_THROUGH_TARGETS) \
	$(PA_THROUGH_TARGETS) $(PA_TEST_THROUGH_TARGETS) $(PA_TEST_TARGETS) \
	$(PA_TEST_NOBUILD_TARGETS)

-include $(foreach checkpoint,$(CHECKPOINTS),$(call pa37_entry_depfile_path,$(checkpoint)))
-include $(foreach src,$(PA37_ALL_SHARED_SOURCES),$(call pa37_shared_depfile_path,$(src)))
-include $(pa37_runner_depfile_path)

FORCE:

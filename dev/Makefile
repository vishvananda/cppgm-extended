DEFAULT_BUILD_JOBS = $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
HOST_UNAME_S = $(shell uname -s)
ifeq ($(findstring -j,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(DEFAULT_BUILD_JOBS)
endif
ifeq ($(MAKELEVEL),0)
CPPGM_MAKE_JOB_FLAGS := $(filter -j% --jobs=%,$(MAKEFLAGS))
CPPGM_MAKE_JOB_LIMIT := $(patsubst --jobs=%,%,$(patsubst -j%,%,$(lastword $(CPPGM_MAKE_JOB_FLAGS))))
ifneq ($(CPPGM_MAKE_JOB_LIMIT),)
CPPGM_MAKE_LOW_JOB_LIMIT = $(shell \
	limit='$(CPPGM_MAKE_JOB_LIMIT)'; cpus='$(DEFAULT_BUILD_JOBS)'; \
	if [ "$$limit" -gt 0 ] 2>/dev/null && [ "$$cpus" -gt "$$limit" ] 2>/dev/null; then echo 1; else echo 0; fi)
ifeq ($(CPPGM_MAKE_LOW_JOB_LIMIT),1)
$(warning make is limited to -j$(CPPGM_MAKE_JOB_LIMIT) on a $(DEFAULT_BUILD_JOBS)-core machine; large compiler builds, especially self-host and PA39/inception builds, will be very slow. Omit -j or use -j$(DEFAULT_BUILD_JOBS).)
endif
endif
endif

TARGETS = \
	abimangle \
	pptoken \
	posttoken \
	ctrlexpr \
	macro \
	preproc \
	recog \
	lowir2cy86 \
	lowiropt \
	lowir2native \
	cppgm++ \
	nsdecl \
	nsinit \
	cy86

DEBUG_TARGETS = $(addsuffix -debug,$(TARGETS))

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
V ?= 0
Q = $(if $(filter 1,$(V)),,@)
quiet = @if [ "$(V)" != "1" ]; then printf '  %-7s %s\n' '$(1)' '$(2)'; fi
CPPGM_STDLIB_FLAGS ?=
ifeq ($(origin CPPGM_HOST_CXX), undefined)
CPPGM_HOST_CXX := $(CXX)
endif
CPPGM_TEST_RUNNER ?= 1
TEST_RUNNER_SHARED_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-DTEST_RUNNER_ENABLE,)
TEST_RUNNER_ENTRY_FLAGS = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),-Dmain=test_runner_real_main,)
HOST_CXX_DEFAULT_DEF = -DCPPGM_DEFAULT_HOST_CXX='"$(CPPGM_HOST_CXX)"'
OBJECT_ROOT_DEFAULT_DEF = -DCPPGM_DEFAULT_OBJECT_ROOT='"$(abspath $(OBJ))"'
CC_FLAGS = -std=gnu++11 -Wall -O3 $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)
DEBUG_CC_FLAGS = -std=gnu++11 -Wall -O0 -g -fno-omit-frame-pointer $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)
ENTRY_CC_FLAGS = $(CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
ENTRY_DEBUG_CC_FLAGS = $(DEBUG_CC_FLAGS) $(TEST_RUNNER_ENTRY_FLAGS)
SRC = ../dev/src
CALLSEMANTIC_SRC = $(wildcard $(SRC)/callsemantic/*.cpp)
OBJ ?= ../obj
GENERATED ?= $(OBJ)/generated
BUILTIN_HOST_CONFIG = $(GENERATED)/cppgm_builtin_host_config.h
INC = -I$(SRC) -I$(GENERATED)
SHARED_OBJ_ROOT = $(abspath ../obj)
CURRENT_OBJ_ROOT = $(abspath $(OBJ))
ifeq ($(CURRENT_OBJ_ROOT),$(SHARED_OBJ_ROOT))
ifneq ($(abspath $(firstword $(CXX))),$(abspath $(firstword $(CPPGM_HOST_CXX))))
$(error shared OBJ root $(OBJ) is only supported with host compiler $(CPPGM_HOST_CXX); use OBJ=<isolated-root> for selfhost or other non-host builds)
endif
endif
VPATH = $(SRC)
RDEPDIR = $(OBJ)/release/.d
DDEPDIR = $(OBJ)/debug/.d

$(shell mkdir -p $(RDEPDIR) $(DDEPDIR) >/dev/null)

FORCE:

$(BUILTIN_HOST_CONFIG): FORCE
	@mkdir -p $(dir $@)
	@CPPGM_BUILD_HOST_CXX='$(CPPGM_HOST_CXX)' \
		CPPGM_STDLIB_FLAGS='$(CPPGM_STDLIB_FLAGS)' \
		perl ./gen_builtin_host_config.pl > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv $@.tmp $@; else rm -f $@.tmp; fi

$(OBJ)/release/preprocessor.o $(OBJ)/debug/preprocessor.o: $(BUILTIN_HOST_CONFIG)

COMMON_SRC_FILES = $(filter-out $(SRC)/test_runner.cpp,$(wildcard $(SRC)/*.cpp) $(CALLSEMANTIC_SRC))
COMMON_OBJ_BASENAMES = $(patsubst $(SRC)/%.cpp,%,$(COMMON_SRC_FILES))
common_objs = $(patsubst %,$(OBJ)/$(1)/%.o,$(COMMON_OBJ_BASENAMES))
runner_obj = $(if $(filter 1,$(CPPGM_TEST_RUNNER)),$(OBJ)/$(1)/test_runner_enabled.o)
link_objs = $(call common_objs,$(1)) $(call runner_obj,$(1))
runner_state_stamp = $(OBJ)/$(1)/.test_runner_mode
link_prereqs = $(call link_objs,$(1)) $(call runner_state_stamp,$(1))
include frontend_source_sets.mk
$(foreach target,$(TARGETS),$(if $(strip $(FRONTEND_OBJ_BASENAMES_$(target))),,$(error missing FRONTEND_OBJ_BASENAMES_$(target) in frontend_source_sets.mk)))
frontend_obj_basenames = $(FRONTEND_OBJ_BASENAMES_$(1))
frontend_link_objs = $(addprefix $(OBJ)/$(1)/,$(addsuffix .o,$(call frontend_obj_basenames,$(2)))) $(call runner_obj,$(1))

all: $(TARGETS)

debug-build: $(DEBUG_TARGETS)

define DEV_FRONTEND_RULES
$(1): $(OBJ)/release $(1).cpp $(call frontend_link_objs,release,$(1)) $(call runner_state_stamp,release)
	$(call quiet,LINK,$$@)
	$(Q)$(CXX) $(ENTRY_CC_FLAGS) $(INC) -o $$@.tmp $$@.cpp $(call frontend_link_objs,release,$(1))
	@if ! cmp -s $$@.tmp $$@ 2>/dev/null; then mv -f $$@.tmp $$@; else rm -f $$@.tmp; fi

$(1)-debug: $(OBJ)/debug $(1).cpp $(call frontend_link_objs,debug,$(1)) $(call runner_state_stamp,debug)
	$(call quiet,LINK,$$@)
	$(Q)$(CXX) $(ENTRY_DEBUG_CC_FLAGS) $(INC) -o $$@.tmp $(1).cpp $(call frontend_link_objs,debug,$(1))
	@if ! cmp -s $$@.tmp $$@ 2>/dev/null; then mv -f $$@.tmp $$@; else rm -f $$@.tmp; fi
endef

$(foreach target,$(TARGETS),$(eval $(call DEV_FRONTEND_RULES,$(target))))

$(OBJ)/release $(OBJ)/debug: %:
	@- mkdir -p $@

FORCE:

$(OBJ)/release/.test_runner_mode $(OBJ)/debug/.test_runner_mode: FORCE | $(OBJ)/release $(OBJ)/debug
	@current=$$(cat $@ 2>/dev/null || true); \
	if [ "$$current" != "$(CPPGM_TEST_RUNNER)" ]; then \
		printf '%s\n' '$(CPPGM_TEST_RUNNER)' > $@; \
	fi

$(OBJ)/release/.compile_config: FORCE | $(OBJ)/release
	@printf '%s\n' "$(CC_FLAGS)" > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

$(OBJ)/debug/.compile_config: FORCE | $(OBJ)/debug
	@printf '%s\n' "$(DEBUG_CC_FLAGS)" > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

$(OBJ)/release/%.o: $(SRC)/%.cpp $(OBJ)/release/.compile_config
	@mkdir -p $(@D) $(dir $(RDEPDIR)/$*)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(CC_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(RDEPDIR)/$*.Td -o $@ $<
	@mv -f $(RDEPDIR)/$*.Td $(RDEPDIR)/$*.d

$(OBJ)/debug/%.o: $(SRC)/%.cpp $(OBJ)/debug/.compile_config
	@mkdir -p $(@D) $(dir $(DDEPDIR)/$*)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(DEBUG_CC_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(DDEPDIR)/$*.Td -o $@ $<
	@mv -f $(DDEPDIR)/$*.Td $(DDEPDIR)/$*.d

$(OBJ)/release/test_runner_enabled.o: $(SRC)/test_runner.cpp $(OBJ)/release/.compile_config
	@mkdir -p $(@D) $(RDEPDIR)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(RDEPDIR)/test_runner_enabled.Td -o $@ $<
	@mv -f $(RDEPDIR)/test_runner_enabled.Td $(RDEPDIR)/test_runner_enabled.d

$(OBJ)/debug/test_runner_enabled.o: $(SRC)/test_runner.cpp $(OBJ)/debug/.compile_config
	@mkdir -p $(@D) $(DDEPDIR)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(DEBUG_CC_FLAGS) $(TEST_RUNNER_SHARED_FLAGS) $(INC) -c -MT $@ -MMD -MP -MF $(DDEPDIR)/test_runner_enabled.Td -o $@ $<
	@mv -f $(DDEPDIR)/test_runner_enabled.Td $(DDEPDIR)/test_runner_enabled.d

clean:
	-rm -f $(TARGETS) $(DEBUG_TARGETS) $(OBJ)/release/*.o $(OBJ)/debug/*.o $(OBJ)/release/.compile_config $(OBJ)/debug/.compile_config

$(RDEPDIR)/%.d: ;
.PRECIOUS: $(RDEPDIR)/%.d

$(DDEPDIR)/%.d: ;
.PRECIOUS: $(DDEPDIR)/%.d

-include $(patsubst %,$(RDEPDIR)/%.d,$(COMMON_OBJ_BASENAMES))
-include $(patsubst %,$(DDEPDIR)/%.d,$(COMMON_OBJ_BASENAMES))
-include $(RDEPDIR)/test_runner_enabled.d
-include $(DDEPDIR)/test_runner_enabled.d

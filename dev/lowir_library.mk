# Independently built PIC objects keep library flags out of normal tool and
# self-host objects. Public C symbols are the only default-visible exports.
LOWIR_LIBRARY_OBJDIR = $(OBJ)/lowir-library
LOWIR_LIBRARY_FLAGS = $(CC_FLAGS) -pthread -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
LOWIR_LIBRARY_OBJS = $(addprefix $(LOWIR_LIBRARY_OBJDIR)/,$(addsuffix .o,$(FRONTEND_LIBRARY_SOURCE_IDS_lowir_library))) $(LOWIR_LIBRARY_OBJDIR)/boundary.o
LOWIR_LIBRARY_SHARED = libcppgm_lowir.$(if $(filter Darwin,$(shell uname -s)),dylib,so)
LOWIR_LIBRARY_LINK = $(if $(filter Darwin,$(shell uname -s)),-dynamiclib,-shared -Wl$(COMMA)-z$(COMMA)defs)
COMMA := ,

.PHONY: lowir-library clean-lowir-library
lowir-library: libcppgm_lowir.a $(LOWIR_LIBRARY_SHARED)

libcppgm_lowir.a: $(LOWIR_LIBRARY_OBJS)
	$(call quiet,AR,$@)
	$(Q)$(AR) rcs $@.tmp $^
	$(Q)mv -f $@.tmp $@

$(LOWIR_LIBRARY_SHARED): $(LOWIR_LIBRARY_OBJS)
	$(call quiet,LINK,$@)
	$(Q)$(CXX) $(CC_FLAGS) -pthread $(LOWIR_LIBRARY_LINK) -o $@.tmp $^
	$(Q)mv -f $@.tmp $@

$(LOWIR_LIBRARY_OBJDIR)/.config: FORCE
	@mkdir -p $(@D)
	@printf '%s\n%s\n' '$(CXX)' '$(LOWIR_LIBRARY_FLAGS)' > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

$(LOWIR_LIBRARY_OBJDIR)/%.o: $(SRC)/%.cpp $(LOWIR_LIBRARY_OBJDIR)/.config
	@mkdir -p $(@D)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(LOWIR_LIBRARY_FLAGS) $(INC) -c -MMD -MP -o $@ $<

$(LOWIR_LIBRARY_OBJDIR)/boundary.o: lowir_library.cpp $(LOWIR_LIBRARY_OBJDIR)/.config
	@mkdir -p $(@D)
	$(call quiet,CXX,$@)
	$(Q)$(CXX) $(LOWIR_LIBRARY_FLAGS) $(INC) -c -MMD -MP -o $@ $<

clean-lowir-library:
	rm -f libcppgm_lowir.a libcppgm_lowir.so libcppgm_lowir.dylib
	rm -rf $(LOWIR_LIBRARY_OBJDIR)

-include $(LOWIR_LIBRARY_OBJS:.o=.d)

.PHONY: test-lowir-library
test-lowir-library: lowir-library cppgm++
	$(Q)cd .. && python3 scripts/test_lowir_library.py

clean: clean-lowir-library

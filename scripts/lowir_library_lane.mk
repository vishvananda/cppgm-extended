# Optional library contract lane, separate from report/test discovery.
.PHONY: test-lowir-library lowir-library-build
lowir-library-build:
ifneq ($(CPPGM_SKIP_DEV_REBUILD),1)
	$(MAKE) -C ../dev lowir-library cppgm++ CXX="$(CXX)" CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" $(if $(strip $(OBJ)),OBJ="$(OBJ)")
endif

test-lowir-library: lowir-library-build
	cd .. && CC="$(CC)" CPPGM_HOST_CXX="$(CPPGM_HOST_CXX)" CPPGM_STDLIB_FLAGS="$(CPPGM_STDLIB_FLAGS)" python3 scripts/test_lowir_library.py --levels $(LOWIR_LIBRARY_LEVELS)

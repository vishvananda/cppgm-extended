# One result namespace per make invocation, inherited by recursive makes.
# An explicit empty value keeps the legacy .my/.check paths for manual use.
ifeq ($(origin CPPGM_TEST_RUN_ID),undefined)
CPPGM_TEST_RUN_ID := $(shell perl -MTime::HiRes=time -e 'printf "%d-%d",time()*1000000,$$$$')
endif
export CPPGM_TEST_RUN_ID
CPPGM_TEST_FAILURE_MARKER := .test_failed$(if $(CPPGM_TEST_RUN_ID),.$(CPPGM_TEST_RUN_ID))
export CPPGM_TEST_FAILURE_MARKER

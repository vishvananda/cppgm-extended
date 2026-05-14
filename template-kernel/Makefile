TARGET ?= ../dev/tmplsolve
TESTS := $(wildcard tests/*.tkq)
OUTDIR := testout

.PHONY: test clean build-target

test: build-target
	@mkdir -p $(OUTDIR)
	@status=0; \
	for testfile in $(TESTS); do \
		base=$$(basename "$$testfile" .tkq); \
		outfile="$(OUTDIR)/$$base.my"; \
		statusfile="$$outfile.exit_status"; \
		reffile="tests/$$base.ref"; \
		refstatus="tests/$$base.ref.exit_status"; \
		printf '== %s ==\n' "$$base"; \
		$(TARGET) -o "$$outfile" "$$testfile"; \
		echo $$? > "$$statusfile"; \
		diff -u "$$reffile" "$$outfile" || status=1; \
		diff -u "$$refstatus" "$$statusfile" || status=1; \
		done; \
	exit $$status

build-target:
	$(MAKE) -C ../dev tmplsolve

clean:
	rm -rf $(OUTDIR)

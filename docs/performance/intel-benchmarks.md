# Refreshing the dedicated Intel measurements

Run on the Intel Linux host with GitHub CLI authentication and Podman or Docker.
Wait for other builds to finish and keep the host quiet during measurement.

Preview the campaign without starting jobs or registering a runner:

```sh
python3 scripts/run_intel_benchmarks_once.py --plan \
  --ref main --revision main
```

Remove `--plan` to start. The seven suites run sequentially: public programs,
self-build/inception timing, index-cpp, simdjson scalar and SSE, Godot, Boost O3, and the internal C++ source subset. Each suite gets a fresh temporary runner and Ubuntu 26.04 container. The
launcher waits for the suite and publication to pass before starting the next;
runner logs and measurements remain in the printed directory. Use `--suite NAME`
to refresh just one suite.

`--ref` selects the workflow branch; `--revision` selects the compiler. The
compiler revision is resolved once for the whole campaign. Workflow tooling and
compiler sources use separate checkouts, so the infrastructure branch can measure
the current `main` compiler without moving `main`. Use `--ref main --revision main` for the current compiler.

Successful runs update the live dedicated **Intel** badges and linked detail
tables on `benchmark-badges`, retaining compact reports and measured commits.
EPYC and hosted Intel keep their own measurements. Failed runs retain diagnostics
in Actions artifacts and do not replace the last successful measurements.

Raw observations, command manifests and build logs stay in the linked Actions
run's artifacts, subject to that workflow's retention period. They are not
committed to Git. See [publication storage](publication-storage.md).

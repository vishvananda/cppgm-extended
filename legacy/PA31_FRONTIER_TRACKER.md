# PA31 Frontier Tracker

Current checkpoint:

- root `make test-report` is now at `1502 / 1503`
- all `< pa31` suites are green again
- targeted PA31 compile fixes now landed for:
  - `609-regex-iterator-difference-alias`
  - `652-member-pointer-traits`
  - `658-istream-static-member-mask-access`
- targeted earlier-PA regressions added for:
  - `pa21/423-class-template-default-cache-isolation`
  - `pa21/424-member-pointer-null-comparison`
  - `pa21/425-explicit-specialization-out-of-class-ctor-replay`
- PA31 expectation/ref cleanup landed for:
  - `657-getline-friend-lambda-access`
  - `661-hosted-max-mixed-arithmetic-deduction`
  - `651-hosted-unordered-map-string-int-link-smoke`

Current remaining PA31 frontier:

- link/runtime: `652-hosted-unordered-set-pointer-link-smoke`

Working order:

1. commit the current link/output checkpoint
2. reproduce `652` outside the harness and keep the linked binary
3. debug the runtime abort with lowir/object inspection and host backtraces
4. add earlier-PA regression tests if the root cause is not PA31-specific
5. rerun full root `make test-report` after the runtime fix

Notes:

- do not refresh PA8 refs from our compiler; PA8 mismatches are compiler regressions
- prefer `make test-report` for broad validation
- prefer PA31 batch harnesses for the current frontier

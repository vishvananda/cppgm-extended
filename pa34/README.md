# CPPGM Programming Assignment 34: Rust compiler reference bootstrap

Complete PA33 first. The course owner supplies a separately frozen snapshot S
of the complete C++ reference implementation. It is a compilation workload,
not an implementation to delegate to or copy into your compiler. Before PA34,
an independent student implementation continues to use only the handouts,
fixtures and supplied reference binaries. At PA34 you may inspect S and reduce
inputs that expose compilation errors; fix those errors in your own compiler.

Create S from a published, pinned cppgm-extended commit, outside the student's
implementation checkout. This exports committed files, records their hashes
and preserves reference-tool links; it includes no local build outputs:

```sh
python3 scripts/reference_bootstrap.py snapshot --repo /path/to/cppgm-extended \
  --revision PUBLISHED_COMMIT --out /path/to/reference-source
```

Use the same bundle revision for runs being compared. Its manifest, full source
lists, generated configuration, flags and host libraries are part of the proof.
The runner refuses changed source inputs and bounds builds to eight jobs.
For Linux use the same GCC host throughout, with its ordinary allocator/link
recipe. Host compilation is used only for the baseline; the candidate must
compile every object in its own generations. Cargo's build time is not C++
compilation throughput.

Let R be your actual release C++ compiler executable (for Rust, the Cargo-built
binary, not a shell/build wrapper), and H the real host C++ compiler:

| Generation | Build | Check |
| --- | --- | --- |
| B | H(S) | Host-built baseline, all nine tools |
| A | R(S) | Complete candidate-built objects and all nine tools |
| C | A(S) | Rebuild with the candidate-built C++ implementation |
| D | C(S) | Corresponding objects and all nine tools equal C |

A need not equal B or the reference's own generated code: independent
implementations may choose different valid code generation. The mandatory
fixed point is C == D. A and D must both pass PA1–PA33 and the PA8/PA32/PA33
debug suites, with all helper tools taken from that generation. Thus a stable
but miscompiled reference implementation does not establish correctness.

```sh
python3 scripts/reference_bootstrap.py progress --bundle /path/to/reference-source \
  --compiler /path/to/release/cppgm --out obj/reference-bootstrap
python3 scripts/reference_bootstrap.py final --bundle /path/to/reference-source \
  --compiler /path/to/release/cppgm --out obj/reference-bootstrap
```

Progress counts successfully compiled source objects and linked tools; it is
not a completed bootstrap proof. The final command performs clean complete
generations and the rebuilt suites. Every generation uses the same effective
source, include and object paths; stored results are isolated and never fill
holes in failed builds. Successful progress builds may be cached, with every
saved object/tool and dependency rechecked. Logs, receipts and `proof.json`
record the exact inputs and results. A nonzero exit means the proof failed.

PA35 optimizes the Rust compiler and the code it generates for this same S.
Keep the successful PA34 proof and the frozen bundle as its starting point.

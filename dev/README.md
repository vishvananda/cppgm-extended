# Compiler implementation

This checkout contains the complete course solution. The student export
replaces tool entry points with scaffolds and supplies only public support
code; students build one cumulative implementation across PA1–PA34.

Keep tool entry points in `dev/` and shared implementation in `dev/src/`.
List each new implementation source, without `.cpp`, in every tool that needs
it in [frontend_source_sets.mk](frontend_source_sets.mk). Assignment wrappers
reuse these source sets.

From the repository root:

```sh
make CXX=g++ CPPGM_HOST_CXX=g++
make test-pa8
```

Keep a real host compiler in `CPPGM_HOST_CXX` during self-host builds. Use a
separate object root for different compiler configurations. Do not commit
build artifacts or generated logs.

See [AGENTS.md](../AGENTS.md) for ownership and required validation, and
[TESTING_AND_REFERENCES.md](../TESTING_AND_REFERENCES.md) for test policy.

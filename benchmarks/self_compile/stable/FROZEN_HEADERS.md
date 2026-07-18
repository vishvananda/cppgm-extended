# Frozen Semantic-Overload Header Closure

`include/` contains the project headers used by the stable
`semantic_overload.cpp` performance workload. The closure was frozen from
`dev/src` at commit `56afe87c1766f41b1f610335d9005a1b1f0538ce` on 2026-07-18.

The 51-file closure was discovered with:

```sh
/usr/local/opt/llvm/bin/clang++ -std=gnu++11 -I dev/src -MM \
  benchmarks/self_compile/stable/semantic_overload.cpp
```

It was then verified by compiling the stable source with `cppgm++` while the
frozen directory was the only project include path. The default performance
gate uses:

```text
-I benchmarks/self_compile/stable/include
```

Do not refresh this directory as part of ordinary compiler work. A deliberate
source or header refresh creates a new benchmark epoch, must update this record,
and requires a new performance baseline. The closure freezes repository project
headers only; host standard-library and compiler-provided headers remain part of
the fixed machine/toolchain environment.

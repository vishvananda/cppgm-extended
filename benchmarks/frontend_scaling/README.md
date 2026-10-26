# Frontend scaling inputs

Generate each input once outside the checkout, then use the identical bytes
and immutable compiler binaries for paired comparisons. These inputs supplement
the frozen self-compile workload; they do not replace or modify its epoch.

```sh
python3 benchmarks/frontend_scaling/generate.py parameter-bases 32768 > /tmp/bases.cpp
dev/cppgm++ -std=c++11 -fsyntax-only /tmp/bases.cpp
```

`parameter-bases` declares independent class templates inheriting their type
parameter. Registering one template's bases must not scan other templates.
Compare sizes 8192, 16384 and 32768 against `plain-templates` at matching sizes
to separate base bookkeeping from the ordinary cost of reading declarations.
Instruction counts should grow approximately linearly; use calibrated paired
blocks for wall-time claims. Retain every observation and check output equality
when comparing compile or AST output.

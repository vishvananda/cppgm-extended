# PA34 Invalid Witness Source Snapshots

These files preserve the invalid pre-repair versions of PA34 hosted/frontier
sources that blocked Clang witness generation during PA34/PA35 convergence.
They are intentionally outside the `pa*` test directories and use `.bad.cpp`
suffixes so no assignment harness collects them as active tests.

After PA34/PA35 convergence, use these as seeds for minimal negative
regressions if `cppgm++` still accepts the invalid forms.

| Snapshot | Original path | Invalid surface |
| --- | --- | --- |
| `506-gnu-decl-specifier-aliases.bad.cpp` | `pa34/tests/compile/506-gnu-decl-specifier-aliases.t` | Non-standard `__unsigned` spelling rejected by Clang/GCC. |
| `520-dependent-decltype-and-transforms.bad.cpp` | `pa34/tests/compile/520-dependent-decltype-and-transforms.t` | Alias template used with too many template arguments. |
| `544-namespace-alias-suffix-attribute.bad.cpp` | `pa34/tests/compile/544-namespace-alias-suffix-attribute.t` | Attribute applied to a namespace alias declaration. |
| `728-hosted-gnu-decltype-local-typedef.bad.cpp` | `pa34/tests/compile/728-hosted-gnu-decltype-local-typedef.t` | Invalid `__decltype__` spelling. |
| `745-class-alias-dependent-destroy-compile.bad.cpp` | `pa34/tests/compile/745-class-alias-dependent-destroy-compile.t` | Function templates redeclared with equivalent defaulted type-template-parameter lists. |
| `HHC-378-typeinfo-address-of-overload-smoke.bad.cpp` | `pa34/tests/frontier/HHC-378-typeinfo-address-of-overload-smoke.t` | Overloaded operator declared with no class or enum parameter. |

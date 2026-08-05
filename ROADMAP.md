# CPPGM Assignment Roadmap

This is the canonical assignment order for the active PA1-PA39 buildout.
Detailed contracts live in each `paN/README.md`; implementation is cumulative
under `dev/` and `dev/src/`.

| Range | Milestones |
| --- | --- |
| PA1-PA9 | preprocessing, recognition, namespace semantics, and CY86 |
| PA10-PA12 | AST, types/lookup, conversions, calls, and overload resolution |
| PA13 | LowIR contract and execution scaffold (`lowir2cy86`) |
| PA14 | typed Itanium ABI name construction (`abimangle`) |
| PA15-PA18 | procedural LowIR lowering, classes, value semantics, and virtual dispatch |
| PA19-PA24 | templates, metaprogramming, constant evaluation, completion, and integration |
| PA25-PA28 | language and object-model closure through virtual inheritance and RTTI |
| PA29 | LowIR-to-native backend (`lowir2native`) |
| PA30 | separate compilation and the `cppgm++` compile/link driver |
| PA31 | host exception metadata and runtime-helper facts |
| PA32-PA33 | host-linkable objects and host C++ ABI/runtime interoperation |
| PA34-PA36 | hosted source/header compatibility and hosted link/runtime behavior |
| PA37-PA38 | LowIR and machine/backend optimization |
| PA39 | staged self-host ladder and inception |

## PA14 ABI Boundary

PA14 owns the typed ABI model and Itanium encoder before the compiler first
emits symbols in PA15. The standalone `abimangle` tool adapts normalized fact
files into that model. Compiler stages construct typed targets and call the
same encoder directly; the fact text format is not an internal compiler
transport.

## Numbering Migration

The 2026-08 move placed the former PA30 `abimangle` assignment at PA14. Former
PA14-PA29 moved forward by one slot; PA1-PA13 and PA31-PA39 did not move. See
[`docs/assignment-numbering-migration-2026-08.md`](docs/assignment-numbering-migration-2026-08.md)
for the exact map and the policy for older historical records.

The detailed pre-migration roadmap proposal is retained under
[`legacy/part2-roadmap-pre-2026-08-renumber.md`](legacy/part2-roadmap-pre-2026-08-renumber.md).

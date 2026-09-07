# September 2026 assignment consolidation

The course now has 34 assignments. This map uses the 39-assignment checkout
at `70a634a73` as its starting point. Original PA13 is retained as PA8: it
introduces the reusable LowIR model, reader/writer, and three required
construction exercises. Original PA9's CY86 assembler is omitted.

| Original assignment | Current assignment and outcome |
| --- | --- |
| PA1 | PA1: preprocessing tokens |
| PA2 | PA2: post-token conversion and literals |
| PA3 | PA3: preprocessing expressions; tool renamed from `ctrlexpr` to `ppexpr` |
| PA4 and PA5 | PA4: one complete `preproc` implementation |
| PA6 | Recognizer product retired; useful syntax observations retained in PA5's parser/AST work |
| PA7 | Namespace-dump product retired; useful type/lookup assertions retained at their semantic owners |
| PA8 | Mock-image product retired; useful declaration, initialization and linking observations retained at their actual owners |
| PA9 | CY86 assembler retired; encoding, fixups and ELF teaching integrated into PA24 |
| PA10–PA12 | PA5–PA7: AST, types/lookup and expression semantics |
| PA13 | PA8: LowIR introduction and required construction exercises |
| PA14 | PA9: typed ABI naming |
| PA15–PA18 | PA10–PA13: procedural lowering and object model |
| PA19–PA24 | PA14–PA19: templates and constant evaluation |
| PA25–PA28 | PA20–PA23: language and object-model completion |
| PA29 | PA24: native backend |
| PA30–PA33 | PA25–PA28: compile/link and host interoperability |
| PA34–PA36 | PA29–PA31: hosted compatibility |
| PA37–PA38 | PA32–PA33: optimization |
| PA39 | PA34: self-hosting and inception |

Every surviving original assignment from PA10 through PA39 subtracts five.
The live roadmap, handouts, wrappers, test routes, reference bundle, and
self-host workflows use the new numbers. The early execution helper remains
the supplied `lowir2native-ref`; students implement their own native backend
at PA24. There is no CY86 assignment or CY86 reference payload.

The [consolidation inventory](early-assignment-consolidation-inventory.tsv)
preserves original case paths, complete input groups, and baseline hashes.
Its destination column uses current paths. Earlier validation entries describe
the numbers under which those commands actually ran.

The [consolidation plan](early-assignment-consolidation-plan.md) and its
[implementation tracker](early-assignment-consolidation-tracker.md) distinguish
the original content migration from the final numbering checks. Other historical
plans and reviews retain their original numbers; original assignment
implementation notes are archived under `docs/implemented/`.

The live placement table is [doc/pa-feature-placement.md](../doc/pa-feature-placement.md).
The older contract-audit tracker retains its historical decisions and counts.
The [August 2026 numbering record](assignment-numbering-migration-2026-08.md)
also retains the numbers used by that earlier migration.

Current self-host checkpoints include:

```sh
make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa34 test-through-pa8 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa34 test-through-pa10 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make inception
```

# WebAssembly references

These pinned upstream documents accompany PA8's target profile and PA37's
wasm32 backend. The assignment's feature and ABI profile defines the required
subset; the core specification also describes features outside that profile.

| Reference | Use |
| --- | --- |
| [Basic C ABI](BasicCABI.md) | wasm32 data layout, argument/result passing and linear-memory stack |
| [Object linking](Linking.md) | linking metadata, relocations, symbols and target features |
| [Binary values](binary-values.rst), [types](binary-types.rst), [instructions](binary-instructions.rst), [modules](binary-modules.rst) | core encoding and section structure |
| [Instruction validation](validation-instructions.rst), [module validation](validation-modules.rst) | operand/control stacks, constant expressions and definition order |

The core files retain their upstream reStructuredText notation and cross
references. The [rendered core specification](https://webassembly.github.io/spec/core/)
is easier to browse. For exact retained sources, revisions, retrieval date and
SHA-256 hashes, see [manifest.json](manifest.json).

The ABI/linking documents retain their upstream
[Artistic License 2.0](conventions-LICENSE). The core documents use the
[W3C Software and Document Notice and License](document-LICENSE); the
[repository license index](spec-LICENSE) identifies its scope. Original
authorship and copyright remain with their upstream contributors.

The runtime, validator and WASI SDK versions/checksums are pinned separately in
`scripts/wasm_tools.json`. They are test tools, not compiler backend
dependencies. No downloaded executables or runtime observations are stored
in this reference bundle.

The pinned linker does not implement every required core/object case. The
[documented limitations](reference-limitations.json) identify those fixtures
and their independent validation/execution checks. They remain required.
Ordinary scalar globals and reference operations are checked with both linkers.
Table-growth checks pass `--growable-table` to LLD to match the assignment's
growable function table.

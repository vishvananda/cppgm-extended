# Imported Symbol Kind Follow-up Plan

## Goal

Carry imported symbol kind explicitly through the lowering/object pipeline so
function imports and object imports are never distinguished by naming
convention or ad hoc split maps.

## Scope

- introduce an explicit imported-symbol kind model (`function` vs `object`)
- thread that metadata through LowIR export/import registration
- make object emission and relocation translation consume the explicit kind
- remove the remaining special-case bookkeeping in `lowirgensemantic`

## Why

The immediate `PA33` host-EH fix split imported runtime functions from imported
objects into separate maps in `lowirgensemantic`. That is enough to remove the
old synthetic helper-table/data-anchor path, but it is still a local
workaround. The long-term design should make imported symbol kind first-class.

## Intended Steps

1. Add explicit symbol-kind metadata near `symbol_linkage::SymbolIdentity` or a
   dedicated imported-symbol record.
2. Replace the `external_function_symbols_` / `external_object_symbols_` split
   in `lowirgensemantic` with one typed import registry.
3. Update object emission to consult the typed import registry directly.
4. Revalidate the `PA33` host-EH lane and one RTTI/vtable import owner on both
   macOS and Linux.

## Validation

- targeted `pa33` EH owners: `191`, `192`, `193`, `194`, `219`, `220`
- targeted `pa33` RTTI/vtable owners that import host symbols
- host and Linux Docker with isolated object roots

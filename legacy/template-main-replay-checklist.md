# Template Main Replay Checklist

This note tracks the misplaced template-boundary work that originally landed in
`/Users/vishvananda/cppgm` and is being replayed into this integration
worktree.

Preserved source branch:

- branch: `codex/template-main-misplaced-20260421`
- checkpoint commit: `d37cb4ae`
- integration cutoff before replay: `653f2c48`
- source replay base on the preserved branch: `9fc2e74b`

Replay rules:

- port every preserved-branch commit from `9fc2e74b` through `d37cb4ae` that
  modifies the semantic/template layer
- if a commit touches files outside that layer, treat it as likely unrelated
  worker work unless proven otherwise
- note the skip here rather than silently dropping it
- adapt the patch to this worktree's current file layout rather than forcing
  the exact original hunk structure

Known structural differences in this worktree:

- there is no `template_instantiation_coordinator.*` layer here
- function/class instantiation already routes through `template_api` request
  structs instead of the coordinator files used in the misplaced checkout

## Replay Status

| Commit | Status | Notes |
| --- | --- | --- |
| `9fc2e74b` | ported | adapted through the existing `template_api` request path instead of the missing coordinator layer; closure definition upgrade now threads explicit `active_owner` and uses explicit class-finalization materialization |
| `4f390c27` | ported | stored out-of-class member definitions now record `owner_output_node`, and specialization matching no longer rediscover owners by lookup |
| `32b21870` | ported | explicit owner identity now threads through instantiation binding/materialization and preferred current-owner selection |
| `2919e0f0` | ported | owner recovery now prefers declared-class lookup instead of broad `lookup_type(...)` |
| `c48817db` | ported | partial source-owner recovery now uses declared-class lookup instead of broad type lookup |
| `7923e853` | subsumed | the affected `template_resolution.cpp` sites already route through `resolve_type_text_without_fragment_fallback(...)` / shared local parsing helpers instead of broad `ctx.lookup_type(...)` |
| `31be1958` | subsumed | unqualified template-id lookup now flows through the later structured/service-side `resolve_type_lookup_text(...)` path and explicit selected materialization hooks |
| `a32833fb` | subsumed | qualified template-id lookup is already handled by the later `QualifiedName` class/alias-template lookup overloads and structured template-id resolution |
| `ea0f41a5` | subsumed | elaborated-class type handling is already covered by the current direct/leaf structured type-lookup path instead of broad semantic lookup |
| `35b4d98a` | ported | alias reparsing now reuses `template_argument_semantics::resolve_type_text_without_fragment_fallback(...)` via a real `TemplateServices` bundle instead of broad `lookup_type(...)` fallback |
| `e91345ce` | ported | services-based type-text resolution now prefers bound/pack/decltype handling before generic type lookup fallback |
| `b38f016a` | ported | added leaf `lookup_non_template_type_name(...)` to the semantic adapter and replayed the narrow template-side call-site use that fits this tree |
| `19910598` | ported | added `QualifiedName` class/alias template lookup overloads and replayed the structured template-template argument lookup path |
| `074ef33c` | ported | replayed the `template_resolution.cpp` structured lookup uses, with an integration-specific guard to keep unqualified names on the unqualified lookup path |
| `1b8e8ae2` | ported | reference-only recursive class-template-id materialization now takes resolved arguments through explicit selected-materialization hooks instead of bouncing back through text argument lookup |
| `68867eb4` | ported | full class-template-id recursive materialization now uses explicit selected-specialization hooks, with selection/materialization split in `template_instantiation` |
| `8b974d9d` | skip-unrelated | modifies `dev/src/symbol_linkage.cpp` and `pa33` tests; not semantic/template boundary work |
| `bb4dd2a3` | ported | added explicit resolved-alias materialization and used it in the narrow semantic alias-template lookup path instead of forcing a second text-based argument resolution pass |
| `4e95634a` | subsumed | this tree's selected-materialization split already routes builtin `initializer_list` instantiation through `instantiate_selected_class_template(...)` and the selected reference/materialization hooks |
| `16ddd3a5` | subsumed | the raw `instantiate_alias_template(...arg_texts...)` fallback had already been eliminated in this tree's services-based template-id resolution path before replay |
| `d642d322` | ported | the remaining `SemanticContext` decltype/typeof parsing path now routes through the services-based fragment-analysis implementation instead of direct `parse_decltype_specifier(...)` |
| `5b2dd8f3` | ported | service-side type-text parsing now goes through shared `parse_type_text_for_templates(...)`, and `template_resolution` call sites use that helper instead of direct fragment parsing |
| `a0df339b` | ported | extracted the shared service-side `parse_type_text_for_templates(...)` implementation and made deduction parsing reuse it |
| `a03de66d` | subsumed | the affected `template_resolution` and `template_instantiation` sites were already on local/template-api argument resolution in this tree before replay |
| `1af7e8d7` | ported | template owner/source-owner recovery now uses leaf non-template type lookup plus `class_info_for_type(...)` instead of broader declared-class lookup |
| `6c16cb59` | subsumed | the local template-argument resolution pieces were already replayed earlier in this tree, and the remaining ctx-era declared-class lookup sites it targeted were no longer present in `template_argument_semantics.cpp` |
| `e7f5c875` | ported | service-side type parsing now tries a local `parse_type_id_ast` hook path before raw fragment fallback, with recursion guards and recursive hook lookup through `parse_type_text_for_templates(...)` |
| `74c0da1a` | ported | the remaining unqualified class/alias-template lookups in `template_resolution.cpp` now use the existing `semantic_lookup::lookup_unqualified_*_template(...)` helpers already present in this tree |
| `ad248e4c` | ported | unqualified source-template recovery in `decompose_template_instantiation(...)` now uses the same local unqualified class-template lookup path |
| `d37cb4ae` | mixed-audited | the boundary-relevant `template_argument_semantics.cpp` pieces were absorbed by the earlier replayed AST-hook/local-lookup work, the `--template-log` docs already live in this tree, and the remaining `template_instantiation.cpp` assignment-operator replay fix is outside the boundary replay scope |

## Obvious Non-Layer Commits

| Commit | Status | Reason |
| --- | --- | --- |
| `8b974d9d` | skip-unrelated | modifies `dev/src/symbol_linkage.cpp` and `pa33` host-mangling tests, not the semantic/template boundary layer |

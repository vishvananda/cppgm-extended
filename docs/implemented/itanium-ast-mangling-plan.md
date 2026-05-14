# AST-First Itanium Mangling Plan

## Goal

Stop fixing ABI symbol mismatches by adding one more spelling parser to
`symbol_linkage.cpp`. The mangler should implement the local Itanium ABI
grammar in `doc/itanium-mangling.txt` from structured compiler data:

- semantic entities for declaration ownership, linkage, ABI tags, constructors,
  destructors, RTTI, thunks, and local entity numbering
- semantic `Type` objects for fully resolved non-dependent type structure
- parser-owned `CppAstNode`, `TemplateIdSyntax`, and `TemplateArgumentSyntax`
  for dependent template signatures and dependent expressions

Rendered C++ text is not a canonical mangling input. Text may remain as a
diagnostic label or as an identifier token stored in structured syntax, but it
must not be reparsed to recover template ids, qualified names, non-type
parameter types, or expression operators.

## Current Problem

The active mangler is centered in `dev/src/symbol_linkage.cpp`. It has grown
several competing paths:

- direct semantic type mangling for concrete types
- AST-based mangling for some dependent result and parameter patterns
- semantic metadata for dependent class/alias template references
- structured syntax with some lexical identifier handling for already-known
  names and substitution keys
- late entry-point recomputation for constructors/destructors that can degrade a
  canonical symbol already computed on the function binding

That mixture is why fixes oscillate. A symbol can be correct at template
instantiation time, then recomputed from a weaker representation during output;
or one test can be fixed by preferring semantic type state while another needs
the original dependent AST spelling.

## Canonical Rule

The mangler chooses one producer per ABI grammar region.

| ABI region | Canonical producer | Allowed fallback |
| --- | --- | --- |
| `<encoding>` / entity ownership | semantic entity + declaration scope | none |
| `<name>` for known entities | semantic declaration name pieces | none |
| `<name>` for dependent syntax | AST qualified-name/template-id syntax | none |
| concrete `<type>` | semantic `Type` tree | none |
| dependent function-template signature `<type>` | AST pattern node plus semantic annotations | concrete semantic type only when the AST node is provably non-dependent |
| `<template-args>` from an instantiation | `TemplateArgument` plus source `TemplateArgumentSyntax` and pack-size metadata | none |
| dependent `<expression>` | `CppAstNode` expression tree | none |
| constructor/destructor entry variants | canonical binding symbol for complete entry; variant rewrite only for base/deleting entry | none |

The important split is dependency-based, matching the Itanium document: concrete
entities and non-dependent values use resolved semantic forms; instantiation-
dependent constructs use the source AST form because the ABI distinguishes
spelling-level structure there.

## Architecture

### 1. Single AST Emitter Facade

Add a small internal facade beside the existing mangler:

```c++
struct AbiAstMangleContext {
  const TypeMangleContext * type_context;
  MangleSubstitutionState * substitutions;
};

bool mangle_type_ast(const CppAstNode & type_id,
                     const TypePtr & actual_type,
                     AbiAstMangleContext & ctx,
                     std::string & out);
bool mangle_template_id_ast(const TemplateIdSyntax & syntax,
                            AbiAstMangleContext & ctx,
                            std::string & out);
bool mangle_template_argument_ast(const TemplateArgumentSyntax & syntax,
                                  const TemplateParameterInfo * parameter,
                                  AbiAstMangleContext & ctx,
                                  std::string & out);
bool mangle_expression_ast(const CppAstNode & expression,
                           AbiAstMangleContext & ctx,
                           std::string & out);
```

At first these functions can delegate to existing structured helpers. The value
is the boundary: callers stop choosing between AST, semantic, and text paths.
The facade becomes the only place where ABI grammar rules are added.

### 2. Structured Producer Requirements

Every migrated call site must have enough source structure before it enters the
mangler.

- `FunctionTemplateDecl` keeps `result_type_pattern` and
  `parameter_declarations_pattern`; missing pattern nodes for a dependent
  signature are producer bugs.
- `TemplateParameterInfo` keeps non-type parameter `decl_specifier_seq`,
  `declarator`, and `abstract_declarator`; the mangler must not reparse
  `non_type_decl_specifier_text`.
- `TemplateArgument` carries `TemplateArgumentSyntax` for type/value/template
  arguments whenever the argument originated from source.
- Template instantiation records carry pack sizes by parameter identity, not by
  guessed rendered argument counts.
- Named semantic types that model dependent class/alias template ids retain
  their template declaration pointer and structured argument syntax.
- Class-template specialization types carry a structured specialization node:
  the template declaration identity, qualified owner, resolved
  `TemplateArgument` vector, source argument syntax when available, parameter
  list, and pack-size metadata. The mangler must emit that structure into the
  current `MangleSubstitutionState`; a cached string encoding is not a valid
  substitute because Itanium substitutions are assigned by the surrounding
  mangling context.

## Implementation Finding: Class Specialization Encodings Are Stateful

An attempted shortcut was to cache every class-template specialization's
Itanium component string during template instantiation and then reuse it when
the type appeared in a later function signature. That fixes some missing-data
cases but is not semantically correct.

For example, PA19
`207-defaulted-nontype-expression-syntax-rewrite` needs clang's substitution
sequence for repeated `iter<int, int *, long int, 1024>` parameter types:

```text
_ZN6holderIiiE14move_and_checkE4iterIiPilLl1024EES3_S3_
```

Reusing a precomputed string emits the right visible template-id spelling, but
it does not register the same intermediate substitutions in the active mangle
state. The repeated parameter then becomes `S1_` or `S4_` depending on nearby
owner handling, both of which differ from clang. Related constructor tests in
PA18, PA21, and PA22 show the same problem for cross-specialization
substitutions such as `duration<...>`, `Box<double>`, and nested `V<...>`
member types.

The implementation path is therefore not "cache more strings". The first slice
introduced a structured class-template-specialization mangling representation
and makes semantic `Type` nodes point at it. The second slice removes the
temporary "force structured" split from emission: once a class-template
specialization has declaration identity and `TemplateArgument` state, that
semantic state is the canonical input for mangling. Member-function owner
prefixes follow the same rule for type and dependent owner arguments: when that
semantic argument state is present, the owner component is emitted from those
arguments instead of reparsing the rendered owner name. Existing concrete
non-type entity arguments still use the legacy spelling path until their symbol
identity is carried as structured semantic data. Cached strings are
acceptable only for context-free encodings such as generated vendor suffixes or
diagnostics, not for ABI grammar regions that participate in substitutions.

The same rule exposed a smaller missing semantic type case: array owner
arguments such as `owner<int *[]>` could mangle a free function parameter from
the semantic `Type`, but member-function owner prefixes fell back to a generated
vendor spelling because the context-free type IR did not model Itanium
`<array-type>` (`A [bound] _ <element type>`). Array types are now emitted from
the semantic `Type` node for known bounds, unknown bounds, and simple dependent
bound names, so owner template arguments do not need to reparse `int *[]` text.

Another important state bug was reset-related rather than parser-related.
Reference-only class template instantiations can be reset when semantic
completion materializes the selected definition. Resetting must clear stale
cached ABI strings, but it must not discard the structured specialization
record on the `Type`: the source template and resolved arguments still define
the same Itanium name, and losing that record sends later function-template
signature mangling back through rendered named text.

Builtin type transforms are treated the same way at the AST boundary. A
transform such as `__remove_reference_t(T)` is not a free-form type string when
the AST node carries the transform operator and a `type_id` child for the
argument. The mangler emits the vendor builtin operator from that AST
annotation and mangles the operand through the normal type-id path. The
legacy rendered-text transform fallback has been deleted; if no AST operand is
available, builtin type-transform mangling fails instead of reparsing the
rendered spelling.

Nested class-template owner prefixes also use the specialization record. A type
such as `tuple<X&&>::Enable<...>` previously entered the structured
specialization path for `Enable`, then immediately converted the owner
`tuple<X&&>` back into text and reparsed `X&&` as a template argument. The
mangler now walks the declaring scope, recognizes when the owner class is
itself a class-template specialization, and emits that owner component from the
owner's `ClassTemplateSpecializationMangleInfo` in the active substitution
state. If an ABI-relevant template-id owner does not carry semantic
specialization data, mangling now fails at that producer boundary instead of
reparsing the owner text.

The same owner identity must also be available when mangling member functions of
non-template nested classes inside an instantiated class template. For a symbol
like `basic_string<char, traits<char>, alloc<char>>::__alloc_traits::f`, the
function's direct owner is `__alloc_traits`, but the ABI prefix still contains
the outer `basic_string<...>` template-id. Function symbol options now carry the
nearest enclosing instantiated class template's name and arguments, and the
nested-name prefix loop uses those arguments for the matching qualifier
component even when that component is not the final qualifier.

Resolved static data-member expressions can expose semantic types that were not
part of the original template-id syntax. In PA22
`407-qualified-member-alias-sfinae`, `same<int, int>::value` resolves to an
integer literal whose type is the anonymous enum that owns `value` inside the
`same<int, int>` specialization. That enum type previously had only the rendered
name `same<int, int>::__anonymous_enum2`, so literal-type mangling reparsed the
owner template arguments from text. Anonymous enum collection now records an
Itanium owner encoding from the enclosing class scope's semantic `Type`, giving
the literal path a structured ABI type and eliminating that template-argument
text fallback without changing the emitted symbol.

Owner class-template arguments are now emitted from semantic
`TemplateArgument` state whenever the owner component is the matching template
component and every concrete value argument can be represented from semantic
data. This covers concrete integral values such as
`helper<1, 0, 2, 4294967295>` and avoids the old heuristic that only switched
to structured arguments for dependent or type arguments. It also covers
function-pointer and object-reference non-type arguments once overload
resolution has selected a concrete `FunctionBinding` or `ValueBinding`: the
mangler emits the Itanium external-name expression from the selected entity's
`SymbolIdentity`, not from the rendered token. Concrete member-pointer,
block-pointer, and pointer-to-object value arguments now require the same
structured selected-entity/value data; unsupported cases fail instead of
falling back to rendered template-argument text.

Member-function symbols now preserve structured owner arguments for every
enclosing class-template component, not just the nearest owner template. That
matters for implicit special members of nested class templates such as
`Outer<int>::Inner<float>::Inner`: the outer `Outer<int>` prefix is emitted from
the outer class specialization's semantic arguments, while `Inner<float>` is
emitted from the inner specialization's arguments. The text prefix path remains
only for ordinary source-name components; template-id owner arguments come from
materialized class-template identity.

Dependent qualified member types now emit preserved member template-id syntax.
For a pattern such as `IterOps<P>::template difference_type<In>`, the owner
type is emitted from its structured dependent class-template state and the
member `difference_type<In>` is emitted from `TemplateIdSyntax`, so the witness
path no longer reparses `P` or `In` from the rendered qualified type text.

Dependent class-template argument conversion now also preserves type identity
when the source argument is already a semantic type, or when the argument syntax
is exactly one of the current template's type parameters. In that second case
the mangler synthesizes the same semantic template-parameter `Type` that the
normal type parser would have produced, so `_Tp` and `_VoidPtr` style owner
arguments are emitted through `try_mangle_type_impl(...)` instead of entering
the template-argument text parser.

Dependent conditional aliases also preserve semantic branch types. For libc++
patterns such as `__conditional_t<_IsConst, typename _Cp::const_pointer,
typename _Cp::pointer>`, the bool condition remains dependent but the two type
branches can still resolve to structured dependent qualified-member `Type`
objects. The deferred alias now stores those `TypePtr`s in its dependent
argument record, and alias argument syntax used by the mangler rebuilds a
semantic `type_id` from that record. The type-name AST path is allowed to use
that structured dependent semantic type, so `_Cp::member` is emitted as a
dependent qualified member instead of being reparsed from template-argument
text.

Static data-member object symbols are likewise built from semantic ownership,
not from `A<B>::value` text. The variable-symbol API now has a static-member
entry point that receives the owning `ClassInfo`, mangles the owner class from
its semantic `Type` in the active substitution state, appends the member source
name, and only falls back to the older qualified-name path if the owner has no
semantic type. This removes the last active template-argument text fallbacks
seen in PA19, PA21, and PA22 static member definitions and references.

Concrete member classes now carry the semantic type of their declaring class.
That matters for hosted libc++ internals such as
`basic_string<...>::__rep` and `__tree<...>::__tree_deleter`: those member
classes are not themselves class-template specializations, but their ABI names
must still include the instantiated outer template. The mangler emits the
member-class nested name from the owner `Type`, reusing the active Itanium
substitution for the owner when clang would do so, instead of reparsing the
rendered `std::__1::...` spelling.

VTT and construction-vtable object symbols are now emitted from `ClassInfo`
semantic types. The old string-entry points have been deleted, and source
semantic output no longer reparses the dynamic/base class-name strings to form
`_ZTT...` or `_ZTC...` symbols.

Primary vtable object symbols use the same type-based special-symbol helper:
semantic output asks `symbol_linkage` for `_ZTV` from the `Type` rather than
hand-splicing a prefix onto a cached encoding string at the call site. The
existing emission guard is unchanged, so this centralizes symbol formation
without broadening external-vtable ownership decisions.

Thread-local wrapper symbols are derived from the already selected object
symbol when one exists. For Itanium C++ globals, that means `_ZTW...` is formed
from the existing `_Z...` encoding instead of reparsing the variable's source
qualified name; the legacy wrapper-name fallback is private to the object-symbol
helper for non-Itanium object names.

Special-member entry variants are derived from the canonical complete-entry
object symbol when one exists. Base constructor/destructor entries and deleting
destructor entries rewrite the Itanium `C1`/`D1` token in the selected object
symbol to `C2`, `D2`, or `D0`; only missing or unrecognized symbols fall back
to the older full function-name mangling path. The same helper is shared by
semantic output and object-backend alias generation so variant handling no
longer has multiple local token rewriters.

Function owner-template components are discovered from the owner class type's
specialization mangle record as well as from `ClassInfo::source_template`.
That matters for instantiated library helper classes whose `ClassInfo` no
longer has direct source-template arguments but whose `Type` still carries the
canonical class-template identity. Member function prefixes for those owners
can now use the preserved `TemplateArgument` vector instead of reparsing the
owner spelling.

Primary vtable export in LowIR collection now also uses the semantic type when
the vtable node carries one. The policy decision for whether a vtable should be
weakly exported to the host ABI still uses the existing external-candidate
predicate, but the `_ZTV...` spelling itself comes from
`vtable_object_symbol_for_type(...)` instead of `node.text`.

The dead public-ish static wrappers around the legacy text manglers have been
deleted (`try_mangle_type_text(...)`, `try_mangle_template_argument_text(...)`,
`try_mangle_component_text(...)`, and similar no-context helpers). The active
ABI paths no longer have audited rendered-text fallback buckets; remaining text
helpers serialize known identifiers, builtin type names, literal values, and
internal symbol keys.

### Remaining Active Text Boundaries

The remaining text-based ABI entry points are now concentrated rather than
spread across the renderer:

- Function symbol identity no longer exposes rendered C++ name entry points.
  Scope-based registration, binding-based
  recomputation, member-function owner names, lambda context encoding, and LowIR
  vtable-entry fallback now pass `QualifiedName` syntax, fail when a semantic
  binding is missing that syntax, or preserve the existing symbol rather than
  reparsing a rendered function name. C-linkage/runtime aliases use an explicit
  C symbol helper.
  The type mangler now guards recursive semantic type graphs directly, so a
  newly structured function-name path fails cleanly when dependent type data is
  cyclic instead of recovering by stack-overflowing into a rendered-name path.
- Variable symbols no longer have a rendered `qualified_name` mangling entry
  point. Namespace/global variable registration walks the semantic `Scope`
  chain and source identifier directly. Static data members require the owning
  `ClassInfo` semantic type. Variable-template specializations or unusual
  variable names now need a structured producer before they can be emitted; they
  do not silently fall back to reparsing text.
- External-vtable decisions and symbol spellings now require semantic class
  type input. The string predicate and `external_vtable_symbol_for_class_name`
  compatibility API have been deleted; callers without semantic class identity
  do not claim a host external-vtable symbol.
- The audited legacy fallback buckets have been deleted. The migrated
  PA18/19/21/22 suite, focused PA32 owner tests, and PA33/PA34 hosted discovery
  tests pass without relying on any audited rendered-text fallback.
  The `template-id-component-text` bucket is gone: template-id mangling now
  consumes preserved `TemplateIdSyntax` and no longer reparses a component
  spelling such as `A<B>` from `node.value`.
  The `builtin-type-transform-text` bucket is gone as well: builtin type
  transforms require an operator annotation plus operand AST/semantic type.
  The standalone `try_mangle_component_text_impl(...)` wrapper is gone; remaining
  component text handling is limited to already-known source-name components and
  substitution keys.

### 3. Text Reparse Audit

The internal audit guard was used to ratchet ABI-relevant rendered-text
fallbacks from allowed compatibility paths to deleted code. The audited buckets
were:

- template-id reparsing from component text
- type-id reparsing from rendered type text
- template-argument reparsing from rendered argument text
- dependent expression operator splitting from rendered expression text
- non-type parameter type reconstruction from declaration text

Those active fallback implementations are now gone.

The guard is no longer active in the implementation. It was a temporary
migration tool, not a student-facing behavior flag; AST-first mangling is now
the default path.

## Implementation Phases

### Phase 0. Baseline and Documentation

- Record this plan.
- Confirm the worktree starts green under the fast direct LowIR strict suite:
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`.
- Confirm PA32 mangling owners:
  `231`, `232`, `233`, `234`, `235`, `236`, `237`, `241`, `242`, `243`, `244`,
  plus the hosted `std::map` copy-assign probe.

### Phase 1. Audit Guard and Facade

- Add an ABI mangling audit helper with a narrow API:
  `note_text_fallback(kind, detail)`.
- During the migration, gate failures with the temporary strict AST mangling
  environment flag.
- Add the AST emitter facade and route existing dependent result/parameter
  callers through it without changing output.
- Acceptance:
  - strict LowIR direct compare for PA18/19/21/22 passes with the guard disabled
  - selected focused mangling tests pass with the guard enabled for the already
    migrated result/parameter boundaries

### Phase 2. Template Ids and Template Arguments

- Move `emit_template_id_component_syntax(...)` behind
  `mangle_template_id_ast(...)`.
- Keep template-id mangling on `TemplateIdSyntax`; if a node only has
  `node.value`, thread the missing `TemplateIdSyntax` from the parser/semantic
  producer.
- Move pack grouping into the template-id AST emitter using
  `TemplateParameterInfo` and instantiation pack-size metadata.
- Add `ClassTemplateSpecializationMangleRef` or equivalent semantic structure
  to class-template specialization `Type` nodes. Emit it through the same
  template-id AST facade so every specialization contributes substitutions in
  the current mangling context instead of replaying a pre-rendered component
  string.
- Acceptance:
  - PA19 `207-defaulted-nontype-expression-syntax-rewrite`
  - PA22 `511-index-sequence-alias-constructor-deduction`
  - PA32 owner/template-template mangling tests `233`, `234`, `236`, `237`

### Phase 3. Dependent Non-Type Template Arguments

- Make non-type template argument mangling consume:
  - the argument expression AST, or
  - an AST-backed simple primary expression for resolved integral values, and
  - structured non-type parameter type syntax from `TemplateParameterInfo`
- Remove active rendered-type fallback calls on non-type parameter type
  strings. This is complete: non-type parameter type mangling now uses the
  preserved declaration syntax or `value_type`.
- Acceptance:
  - PA19 `203-dependent-nontype-template-arg-mangle`
  - PA19 `205-nontype-pack-comma-expression-syntax`
  - PA19 `207-defaulted-nontype-expression-syntax-rewrite`
  - PA32 `232-host-dependent-nontype-expression-mangling`
  - PA32 `235-host-dependent-bool-owner-argument-mangling`

### Phase 4. Dependent Expressions

- Implement expression AST cases directly from the Itanium operator grammar:
  literals, template parameter references, function parameter references,
  unary/binary/ternary operators, calls, casts, `sizeof`, `sizeof...`,
  `alignof`, member access, qualified static member references, and decltype
  operands.
- Remove active expression mangling by splitting rendered text.
- Acceptance:
  - PA22 `403-dependent-decltype-call-pack-expansion`
  - PA22 `404-dependent-decltype-nested-call-pack-expansion`
  - PA22 `493-pack-expanded-enable-if-member-value`
  - PA32 `236-host-dependent-enable-if-alias-expression-mangling`
  - PA32 `242-host-builtin-transform-alias-mangling`

### Phase 5. Entity Names and Entry Points

- Replace `try_mangle_named_text(...)` for known semantic entities with
  structured entity-name components.
- Split this into ratcheted subphases:
  - `5a`: vtable/RTTI policy entry points use semantic class `Type` input when
    available; text predicates remain only for legacy no-type callers. Done
    for LowIR external-vtable decisions, semantic vtable-output policy, and
    primary vtable export. The remaining no-type vtable text fallback has now
    been removed.
  - `5b`: variable-template and unusual static/global variable symbols carry a
    structured owner/name/template-argument record instead of falling back to a
    rendered `qualified_name`.
  - `5c`: function symbol identity receives a semantic entity-name record
    containing declaration scope, owner class/type, identifier/operator kind,
    ABI tags, lambda context, and constructor/destructor kind. The old
    `qualified_name` entry point becomes a compatibility wrapper and then is
    deleted once strict coverage no longer needs it. In progress: function
    registration, binding-symbol recomputation, destructor entry recomputation,
    member-function owner names, lambda context encoding, and LowIR
    vtable-entry fallback now use `QualifiedName` syntax when the producer has
    it.
- Keep complete constructor/destructor output canonical: if a binding already
  has the complete object symbol, complete output uses that symbol. Only base
  and deleting variants rewrite the ctor/dtor code.
- Add explicit checks for lambda context/signature substitutions and RTTI names.
- Acceptance:
  - PA32 `241-host-member-lambda-mangling`
  - PA32 `243-host-lambda-rtti-mangling`
  - PA32 `244-host-nested-lambda-parameter-substitution-mangling`
  - existing constructor-symbol retention/absence tests around PA32 `245`-`248`

### Phase 6. Delete Text Fallback Parsers

- The active audited text fallback implementations are gone:
  `function-name-text`, `template-id-component-text`,
  `builtin-type-transform-text`, `type-text`, `template-argument-text`, and
  dependent expression text. C/runtime aliases use explicit C-linkage symbol
  helpers instead.
- Keep only lexical string serialization for already-known identifiers, such as
  Itanium `source-name` length fragments and internal-symbol normalization.
- Acceptance:
  - strict LowIR direct compare for PA18/19/21/22
  - PA32 mangling/host-ABI owners
  - PA33/PA34 hosted discovery report

### Phase 7. Ratchet

- Treat AST-first mangling as the default behavior for migrated ABI boundaries.
- Delete dead text reparse helpers once no active call sites remain.
- Add a checklist to the follow-up tracker for unsupported Itanium grammar
  constructs that are outside current course/compiler coverage, rather than
  silently accepting them through text parsing.
- Current boundary: PA18/19/21/22 direct LowIR strict tests pass without the
  retired migration flag, and the focused PA32 mangling-owner tests, including
  the hosted `std::map` copy-assignment probe, pass on the default path. The
  dependent libc++ SFINAE default argument `_Templ<_Args...>` used by
  `__sfinae_test_impl` carries its original `TemplateArgumentSyntax` through
  template argument resolution and deduction finalization, so it mangles from
  AST syntax instead of reparsing the old rendered spelling
  `typename typename _Args...`. The remaining concrete libc++ owner cases are
  handled by member-class owner metadata, so the hosted `std::map` probe is now
  inside the default AST-first mangling boundary.
- External vtable address emission now consumes the semantic class type carried
  by `vptr_action` and RTTI candidates. Concrete class-template specializations
  also carry a copied template identity (scope prefix, template name, and
  parameter list), so hosted libc++ vtable symbols such as stream
  specializations are formed from stable semantic data instead of reparsing the
  rendered class name or dereferencing stale template declaration scopes during
  LowIR emission.
- Hosted VTT parameter references now carry the semantic VTT owner type and bind
  internal VTT placeholders to the semantic `_ZTT...` object symbol when the VTT
  is supplied by the host library. Namespace-scope variable mangling also emits
  the Itanium `St` substitution for semantic `std` scopes, so references such as
  `std::__1::cout` and `std::__1::cerr` match clang/libc++ spelling.
- Vtable external-candidate policy, scope-registered function symbols, binding
  symbol recomputation, destructor entry recomputation, and LowIR vtable-entry
  fallback now consume semantic type or `QualifiedName` inputs when available.
  C-linkage/runtime aliases use the explicit C symbol helper.

## Testing Strategy

Run the fast suite after every committed phase:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict \
  STRICT_PAS='pa18 pa19 pa21 pa22' \
  STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Run PA32 mangling owners after every phase that touches object symbols:

```sh
for t in \
  tests/spec/231-host-dependent-alias-template-mangling.t \
  tests/spec/232-host-dependent-nontype-expression-mangling.t \
  tests/spec/233-host-template-template-parameter-mangling.t \
  tests/spec/234-host-owner-pack-reference-mangling.t \
  tests/spec/235-host-dependent-bool-owner-argument-mangling.t \
  tests/spec/236-host-dependent-template-template-parameter-mangling.t \
  tests/spec/236-host-dependent-enable-if-alias-expression-mangling.t \
  tests/spec/237-host-array-owner-template-mangling.t \
  tests/spec/237-host-dependent-qualified-member-owner-mangling.t \
  tests/spec/241-host-member-lambda-mangling.t \
  tests/spec/242-host-builtin-transform-alias-mangling.t \
  tests/spec/243-host-lambda-rtti-mangling.t \
  tests/spec/244-host-nested-lambda-parameter-substitution-mangling.t \
  tests/spec/235-host-std-map-copy-assign-empty-comparator.t
do
  make -C pa32 check TEST="$t" \
    CXX=/usr/local/opt/llvm/bin/clang++ \
    CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
    CPPGM_SKIP_DEV_REBUILD=1 || exit 1
done
```

Run that PA32 list, including the hosted `std::map` probe, on the default
mangling path. The hosted probe now serves as the current ratchet for libc++
member-class owner metadata and should not require a compatibility fallback.

When a direct LowIR ref changes, verify the mangled name against clang at `-O0`
before updating the ref. A ref update is valid only when the new symbol matches
the Itanium spec and clang's spelling for the same source.

## Definition of Done

- No migrated ABI-relevant path reparses rendered C++ text.
- A missing AST node in a dependent signature causes a targeted producer failure
  during mangling instead of entering a rendered-text compatibility path.
- PA18/19/21/22 direct LowIR strict suite passes without semantic-fallback
  flags.
- PA32 mangling owners pass.
- Remaining text helpers are either dead, used only for non-ABI diagnostics, or
  listed explicitly as unsupported follow-up work.

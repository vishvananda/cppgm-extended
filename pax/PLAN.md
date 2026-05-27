# PAX ABI Naming Plan

## Recovered Proposal

The earlier proposal was to frame this as an ABI naming assignment, not as
"mangling arbitrary C++ source." The important split is:

1. Mangler core: given a normalized ABI entity, type, template, and context
   description, emit the exact Itanium name.
2. Compiler integration: given real semantic state, produce that normalized ABI
   description for declarations the compiler emits.

Only testing C++ source to mangled names keeps the assignment tied to each
implementation's semantic internals. Only testing a standalone DSL lets an
implementation pass while still failing integration. PAX should include both,
but weight the standalone ABI fact tests heavily.

## Placement

The best final insertion point is between PA30 and PA31.

PA30 already has the full source-to-object/link driver shape, but it uses the
course object/link model. PA31 asks for ordinary host-linkable object files.
Correct host symbol names are a prerequisite for PA31, but they are not the
same problem as ELF/Mach-O object emission, section ownership, relocation
selection, archives, or host driver interoperability.

Keeping PAX before PA31 gives the sequence:

1. PA30: compile/link driver over the course object model.
2. PAX: host ABI names from typed semantic facts.
3. PA31: host object/toolchain interoperability using those names.
4. PA32: host C++ ABI/runtime interoperability.

Until the numbering is settled, `pax/` is intentionally not wired into the root
assignment list.

## Implementation Plan

## Typed Model Refactor Plan

The ABI fact model should become the representation consumed by the Itanium
encoder. The parser may keep a line-oriented syntax, but after parsing the
implementation must not translate into a second internal ABI IR just to encode
the result.

The refactor is intentionally staged:

1. Split parsed fact syntax from resolved ABI facts.
   - The text format can continue to use names such as `T_arg` or `String`.
   - A resolver should convert those parser IDs into typed handles once.
   - Encoder entry points should receive only resolved `AbiType`,
     `AbiTemplateArg`, `AbiDependentExpr`, `AbiEntity`, and `AbiFunction`
     records.

2. Replace semantic string payloads with enums and typed fields.
   - Builtin ABI type spellings become `AbiBuiltinType`.
   - Standard-library substitutions become `AbiStdSubstitution`.
   - Dependent-expression operator spellings become operator enums.
   - Function terminals and special names remain typed terminal enums.
   - Array bounds become typed integer/expression bounds instead of raw
     encoded text.
   - Raw strings remain only where the ABI grammar genuinely consumes source
     spellings: identifiers, ABI tags, discriminators, external raw symbols,
     and literal values.

3. Replace string references with compact handles in resolved records.
   - `AbiTypeId`, `AbiTemplateArgId`, `AbiExprId`, `AbiEntityId`, and
     `AbiContextId` should be stable indexes into resolved vectors.
   - Parser IDs should not cross into the encoder.

4. Make substitutions typed.
   - Remove structural string payloads from substitution keys.
   - Use a tagged `AbiSubstitutionKey` with nested typed keys.
   - Keep substitution ordering in one `AbiMangleContext` owned by the encoder.

5. Encode directly from the ABI model.
   - Move type, template-argument, expression, function, and special-name
     emission onto the ABI records.
   - `abimangle` and future `cppgm++ --emit-abi-names` should call this same
     encoder.
   - The old `itanium_mangle_ir` layer can then be reduced to implementation
     helpers or removed from the fact tool entirely.

6. Optimize ownership once the representation is direct.
   - Store resolved records in vectors and pass compact IDs or const
     references.
   - Avoid copying type/template vectors while resolving facts.
   - Move parsed vectors into resolved records where no later serialization
     needs the original object.

7. Migrate production compiler callers incrementally.
   - First add semantic-to-ABI-model builders beside the existing
     `symbol_linkage` lowering path.
   - Compare outputs for focused PA31/PA32 cases.
   - Then switch symbol generation to the ABI model and delete the duplicate
     direct `itanium_mangle_ir` construction.

The implementation slice in this branch makes `abimangle` encode from the ABI
fact records directly, replacing stringly builtin, vendor qualifier,
standard-substitution, dependent-operator, and array-bound fields where the
current tests expose them. The fact encoder also checks references before use
and emits member-owner prefixes structurally instead of slicing encoded owner
strings. Parser IDs remain only as fact-file linking syntax; future source
integration should lower directly into the same typed records instead of
constructing or reparsing those fact IDs.

The larger `symbol_linkage` migration should be done as follow-up slices so
each semantic mangling regression remains bisectable.

### Step 1: Standalone ABI Model

Add a small typed ABI naming library under `dev/src/`, likely split as:

- `abi_name_model.{h,cpp}` for `AbiEntity`, `AbiType`, `AbiTemplateParam`,
  `AbiTemplateArg`, and context records
- `abi_fact_parser.{h,cpp}` for the normalized fact input
- `itanium_abi_mangler.{h,cpp}` for encoding and substitution-table behavior
- `abimangle.cpp` as the standalone assignment tool

The first tests should use fact files only and cover ordinary names, builtin
types, qualified names, pointers/references, function parameters, and C linkage.

### Step 2: Substitutions And Template Facts

Extend the model and standalone tests for:

- substitution table insertion and reuse
- standard substitutions
- template type parameters and non-type template parameters
- default template arguments that appear in names
- packs and template-template parameters
- dependent names, aliases, return types, and expressions

This is the highest-value part of PAX because these are the cases that caused
PA31/PA32 agents to drift into string heuristics.

### Step 3: Special Names

Add standalone coverage for the special name forms later object assignments
need:

- constructors and destructors, including complete/base/deleting entry points
- overloaded operators and conversion functions
- local entities, lambdas, anonymous namespaces, inline namespaces, and ABI tags
- typeinfo, vtables, VTTs, and thunks

The assignment should require names for these entities, not object layout or
runtime behavior.

### Step 4: Source Integration

Add `cppgm++ --emit-abi-names`. This should walk the real semantic model and
emit a stable table of ABI-visible entities:

- defined strong/weak symbols
- undefined external references
- selected special names such as typeinfo/vtables only when the frontend has
  enough semantic ownership facts to request them

This mode must build `AbiEntity` records directly from semantic bindings and
type records. It should not parse rendered LowIR, rendered C++ text, demangler
output, object files, or `nm` output.

### Step 5: Reuse From Host Object Emission

Refactor existing `symbol_linkage` code toward the shared ABI model. The object
backend should ask for an object symbol from semantic/LowIR metadata that was
ultimately produced by PAX's mangler, rather than carrying a separate pile of
Itanium-specific string code.

## Suggested Test Slices

- `100-basic`: functions, variables, namespaces, C linkage, builtin types,
  cv-qualification, pointers, references, arrays, function types, and member
  pointers
- `200-substitutions`: nested-name substitution, repeated type substitution,
  repeated template argument substitution, standard substitutions, inline
  namespace interactions, and ABI tags
- `300-templates`: type/NTTP/template-template parameters, packs, dependent
  aliases, `decltype`, dependent returns, default arguments, and owner-template
  contexts
- `400-special-names`: constructors, destructors, operators, conversions,
  local entities, lambdas, typeinfo, vtables, VTTs, and thunks
- `500-source-integration`: representative C++ inputs that prove the compiler's
  semantic model feeds the same ABI records as the standalone facts

## Existing Tests To Consider Moving Earlier

Representative PA31/PA32 tests that are mostly ABI-name pressure and could
inform PAX coverage:

- `pa31/tests/general/200-synthetic-std-hard-substitution-mangling.t`
- `pa31/tests/general/200-synthetic-template-arg-substitution-mangling.t`
- `pa31/tests/general/200-synthetic-inline-namespace-template-mangling.t`
- `pa31/tests/general/200-function-template-type-param-substitution.t`
- `pa31/tests/general/200-function-template-nested-ref-substitution.t`
- `pa31/tests/general/200-dependent-nttp-default-return-mangling.t`
- `pa31/tests/general/200-dependent-alias-return-decltype-operator-mangling.t`
- `pa31/tests/general/200-host-class-template-member-dependent-return-mangling.t`
- `pa31/tests/general/200-host-member-pointer-owner-template-mangling.t`
- `pa31/tests/general/200-synthetic-member-operator-function-pointer-mangling.t`
- `pa31/tests/general/200-namespace-operator-template-std-string-substitution.t`
- `pa32/tests/general/200-host-std-template-substitution-slots.t`
- `pa32/tests/general/200-host-template-template-parameter-mangling.t`
- `pa32/tests/general/200-host-dependent-template-template-parameter-mangling.t`
- `pa32/tests/general/200-host-dependent-nontype-expression-mangling.t`
- `pa32/tests/general/200-host-dependent-enable-if-alias-expression-mangling.t`
- `pa32/tests/general/200-host-dependent-qualified-member-owner-mangling.t`
- `pa32/tests/general/200-host-nested-lambda-parameter-substitution-mangling.t`
- `pa32/tests/general/200-host-array-owner-template-mangling.t`
- `pa32/tests/general/200-host-builtin-transform-alias-mangling.t`

Some of these should remain as PA31/PA32 end-to-end host object checks, but PAX
should cover their name-construction core before host object emission is graded.

## Reference Strategy

PAX should use checked-in `.ref` files. Instructor tooling may generate or audit
those references with a host compiler, but student test execution should not
shell out to a host compiler as the oracle.

The reason is pragmatic: host compilers and standard libraries differ in ABI
tag spellings, inline namespace names, and local numbering details. The
assignment should define the course ABI subset explicitly and test that stable
subset.

## Risks

- The ABI fact format can accidentally become a second C++ frontend. Keep it as
  a normalized entity graph with no parsing of expressions beyond the forms the
  ABI grammar needs to encode.
- Source integration can become a string bridge. Make the README explicit that
  semantic records feed the ABI model directly.
- Some PA32 special names depend on layout or ownership facts that are not fully
  known before host ABI/runtime work. PAX should name only the special entities
  whose semantic facts are already available, and later PAs can add more source
  integration coverage while reusing the same mangler.
- Existing `symbol_linkage` code is large and intertwined with semantic helper
  types. The first branch should establish the assignment and model boundary;
  the implementation branch can refactor incrementally.

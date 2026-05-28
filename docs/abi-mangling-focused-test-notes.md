# ABI mangling focused test notes

Working notes from the string-fallback removal branch. These are the cases that
were exposed while making symbol linkage go through typed ABI IR only, and they
should become focused mangling tests in the ABI assignment branch.

## Fixed in this branch

- `pa26/tests/general/300-lambda-rtti-typeinfo-name.t`
  - Gap: local lambda closure type substitution keys were not emitted through the
    typed IR path.
  - Focused test: mangle a function-local lambda closure type and its RTTI/name
    uses.

- `pa27/tests/general/100-capturing-lambda-local.t`
  - Gap: templated/generic local lambda metadata was not assigned early enough
    for closure special members.
  - Focused test: mangle a local capturing lambda closure with generated special
    members.

- `pa28/tests/general/300-member-pointer-nontype-template-parameter.t`
  - Gap: data member pointer NTTP values were not preserved as value bindings for
    typed ABI IR emission.
  - Focused test: mangle a template specialization with `&Class::member` as an
    NTTP.

- `pa28/tests/general/300-member-function-pointer-nontype-partial-specialization-call.t`
  - Gap: member function pointer NTTP payloads did not build an external entity
    expression through typed ABI IR.
  - Focused test: mangle a partial specialization selected by
    `&Class::member_function`.

- `pa33/tests/general/200-host-member-lambda-mangling.t`
  - Gap: lambda closure constructors/destructors needed typed closure
    substitution keys and constructor/destructor terminal fragments.
  - Focused test: mangle a lambda closure nested in a member-function context.

- `pa34/tests/compile/500-compressed-pair-padding-instantiation.t`
  - Gap: dependent named syntax such as allocator `rebind` needed to preserve
    the dependent qualified type rather than falling back to text ABI names.
  - Focused test: mangle a dependent nested template-id type like
    `allocator<T>::rebind<U>::other`.

- `pa34/tests/compile/600-catch-type-id-identifier.t`
  - Gap: dependent alias/type-id forms needed to emit from the typed dependent
    syntax path.
  - Focused test: mangle a dependent alias used as a type-id.

- `pa34/tests/compile/600-hosted-vector-bool-storage-allocator-static-cast.t`
  - Gap: member class-template specialization owners needed to be built from the
    current qualified owner instead of being re-resolved as namespace templates.
  - Focused test: mangle a nested member class-template specialization with a
    dependent owner.

- `pa34/tests/compile/600-const-unordered-map-find.t`
  - Gap: dependent alias named keys needed the direct typed syntax path.
  - Focused test: mangle a dependent alias key used in an STL-like iterator or
    allocator type.

## Additional fixed cases

- `pa34/tests/compile/600-regex-iterator-difference-alias.t`
  - Gap: weak symbol
    `std::_Base_bitset<((_Nb)/(8*8)+((_Nb)%(8*8)==0?0:1))>::_Base_bitset`.
  - Fix area: dependent non-type expression template arguments with arithmetic,
    comparison, and conditional expression pieces now use typed expression AST
    lowering instead of reparsing text.
  - Focused test: mangle a class-template specialization whose NTTP argument is
    an expression like `N / 64 + (N % 64 == 0 ? 0 : 1)`.

- `pa34/tests/compile/700-hosted-function-template-default-allocator-local-lambda-compile.t`
  - Gap: weak symbol
    `std::vector<Info const *, std::allocator<Info const *>>::_Guard_alloc::_Guard_alloc`.
  - Fix area: special member symbol for a nested helper class inside a
    concrete class-template specialization.
  - Focused test: mangle a nested non-template helper class special member owned
    by `Outer<T, Alloc<T>>`.

- `pa35/tests/link/600-hosted-unordered-set-pointer-link-smoke.t`
- `pa35/tests/link/700-hosted-namespace-alias-pointer-template-arg.t`
- `pa35/tests/link/700-hosted-vector-string-pushback-link-smoke.t`
  - Gap: dependent-qualified member template metadata kept the full qualified
    template-id even though the typed owner had already been built.
  - Focused test: mangle a dependent-qualified member template type such as
    `Owner<T>::template rebind<U>::other` where the owner is already typed.

- `pa22/tests/general/400-function-assignment-invocable-and-helper.t`
- `pa22/tests/general/500-base-qualified-template-value-arg-syntax.t`
- `pa22/tests/general/500-internal-remove-cvref-alias-sfinae.t`
- `pa34/tests/compile/500-local-functor-std-function-assignment.t`
- `pa34/tests/compile/700-hosted-local-class-distinct-member-symbols-compile.t`
  - Gap: named function-local class ordinary member functions, including
    `operator()` and `operator=`, were not using typed local-entity metadata.
  - Focused test: mangle ordinary members and fixed operators for a named local
    class.

- `pa35/tests/link/700-hosted-getline-indirect-result-vbptr-link-smoke.t`
- `pa35/tests/link/700-hosted-iostream-nounitbuf-string-runtime-smoke.t`
- `pa35/tests/link/700-hosted-istream-ref-getline-eof-runtime-smoke.t`
  - Gap: unqualified template-ids inside function template parameter patterns
    did not search inline namespace children, so `basic_string` missed the
    typed `std::__cxx11` owner.
  - Focused test: mangle a function template taking
    `basic_string<Char, Traits, Alloc>&` from an enclosing namespace with an
    inline child namespace.

- `pa35/tests/link/700-hosted-local-class-template-mangling-link-smoke.t`
- `pa35/tests/link/700-nested-template-local-owner-symbol-link-smoke.t`
  - Gap: expectations still used the old synthetic local-class names after the
    typed local-entity ABI spelling was restored.
  - Focused test: mangle a function template instantiated with a function-local
    class, and a nested template owner containing a function-local class.

- `pa34/tests/compile/600-hosted-deque-member-template-include.t`
  - Gap: libc++ owner-template self references such as
    `__deque_iterator<..., _BlockSize>::__block_size` could spell the owner
    template argument with the dependent value text instead of matching the
    current owner template parameter slot.
  - Focused test: mangle a class-template static data member and special member
    where the final NTTP owner argument is the same dependent template
    parameter.

- clang/libc++ test-through hosted bitset/deque owners
  - Gap: current-class lookup could reject a named type that was exactly the
    active class type, which blocked typed ABI IR for libc++ member owners such
    as `__bitset<_Size == 0 ? ...>::__bitset`.
  - Focused test: mangle a special member of a class-template owner whose
    dependent NTTP argument is a conditional expression from the owner parameter
    list.

- `pa34/tests/compile/600-hosted-forward-as-tuple-rvalue-ref.t`
  - Gap: alias-template substitution replaced `_Tp` inside a type-id by editing
    only the leaf text. That dropped the template-id/semantic payload for
    arguments such as `__remove_cvref_t<_Tp>`, so nested libc++ aliases like
    `_IsNotSame<__remove_cvref_t<_Tp>, __tuple_leaf>` could not lower
    `!__is_same(...)` through typed ABI IR.
  - Focused test: mangle an `enable_if` NTTP default whose condition goes
    through nested alias templates and a builtin type trait expression.

- clang/libc++ self-host `semantic_model.o`
  - Gap: libc++ C11 atomic support instantiated `std::addressof` on
    `_Atomic(unsigned long)`, but typed ABI IR did not model `_Atomic(T)`.
  - Focused test: mangle a function template instantiated with `_Atomic(T)`,
    which should encode as an Itanium vendor-qualified type
    (`U7_Atomic...`).

- clang/libc++ self-host `parser_trace.o`
  - Gap: libc++ `basic_string::compare(pos, n, basic_string, pos, n)`
    forwards to a hidden `basic_string_view` member-template overload. The
    in-class declaration used injected-class-name `basic_string` inside the
    SFINAE NTTP default, while the out-of-class definition spelled the current
    specialization as `basic_string<_CharT, _Traits, _Allocator>`. The
    definition matcher treated those as different template heads, so the
    member-template definition was not attached and the object referenced an
    unresolved hidden libc++ symbol.
  - Focused test: associate an out-of-class member-template definition whose
    template-parameter list compares an injected class name against the explicit
    current class-template specialization spelling.

- `pa14/tests/general/200-lvalue-conditional-address.t`
- `pa32/tests/general/200-host-namespaced-enum-template-arg-mangling.t`
- `pa35/tests/link/700-hosted-iostream-runtime-symbol-link-smoke.t`
  - Gap: public non-weak functions suppressed parameter-type substitution keys
    too aggressively. The old behavior happened to fit weak emission, but
    ordinary external functions need complete parameter substitutions so later
    arguments can reuse earlier parameter encodings.
  - Focused test: mangle a public overloaded function where a later parameter
    should substitute a namespace-qualified or template-qualified earlier
    parameter.

- `pa18/tests/general/300-function-template-pack-ref-return.t`
  - Gap: manual function-template mangling registered pack-expansion
    substitutions through a rendered string key. After the string key was
    removed, pack expansion parameters needed a typed substitution key that
    matches the emitted `Dp` + inner type sequence.
  - Focused test: mangle a function template whose trailing function parameter
    pack is later reused by reference in the result or parameter list.

- `pa18/tests/general/300-inline-dependent-pack-result-type.t`
- `pa21/tests/general/400-explicit-type-arg-dependent-template-cv-suffix.t`
- `pa21/tests/general/400-local-qualified-argument-replay.t`
- `pa22/tests/general/500-type-pack-element-preserves-concrete-argument.t`
- `pa22/tests/spec/400-explicit-type-arg-dependent-qualified-member-template-id.t`
- `pa28/tests/general/300-member-function-pointer-pack-partial-specialization.t`
  - Gap: copied template-parameter lists used pointer identity as part of the
    typed substitution key. Replayed contexts and copied lambda/member-template
    contexts need stable keys derived from the parameter-list shape instead.
  - Focused test: mangle copied/replayed template contexts where the same
    parameter slot appears in dependent owners, cv-qualified suffixes, pack
    elements, local contexts, and member-function-pointer NTTPs.

- `pa22/tests/general/200-explicit-function-template-arg-substitution.t`
  - Gap: dependent result mangling sometimes needed typed result emission to
    substitute an earlier parameter, but broad typed result emission also caused
    incorrect source-name spellings for unresolved dependent names.
  - Focused test: mangle a function-template specialization where the return
    type structurally matches an earlier parameter type and must substitute it,
    plus a neighboring case where unresolved dependent names must stay in the
    existing expression-pattern path.

- `pa26/tests/general/200-included-template-member-dual-capturing-lambda-call.t`
- `pa27/tests/general/200-distinct-lambda-member-template-types.t`
  - Gap: local entity context replay should emit template-parameter types
    directly inside the replayed context while still registering substitutions
    for later ABI components. Normal function type emission should keep the
    usual substitution behavior.
  - Focused test: mangle local lambda/member-template contexts that include
    direct template-parameter parameter types and then reuse those parameters
    outside the local entity context.

## Current status

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` passes all tests:
  `2992 / 2992`.

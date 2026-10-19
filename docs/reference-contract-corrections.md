# Reference contract corrections

These changes fix the compiler and comparison harness that produce or grade
course references. References are regenerated with the built solution; source
fixtures and executable outcomes are preserved. The rules apply across PAs.

## Compiler producers

- Logical value normalization compares each integer in its actual operand type,
  while the comparison result remains `i64` (PA8 LowIR comparisons).
- Compound assignments evaluate and convert the RHS before obtaining the LHS
  address and reading it, following the course's explicit evaluation convention.
  This includes atomic updates and bfloat excess-precision operations.
- A TLS object's address is obtained by runtime initialization, rather than a
  process-wide data relocation, through both constant-initializer paths.
- Constant and dynamically initialized reference temporaries retain their type's
  alignment even when their backing data consists entirely of zero bytes.
- A void-cast reference-returning call evaluates the call without an extra load;
  calls are not one of N3485 5p11's discarded volatile-glvalue read forms.
- Explicit instantiation after explicit specialization adds no emission demand
  (14.7.2p5). Complete empty value-initialized objects include their padding byte
  in zero-initialization (8.5p6); overlapping empty subobjects remain protected.
- Exception cleanup dispatches explicitly retain records that their shared
  suffixes retire. Construction-prefix handlers use persistent cleanup regions.
  Detached constructor/destructor bodies retain the call record before entering
  a suffix that removes it and the body record, preserving host EH execution.
- Failed reference `dynamic_cast` and null dynamic `typeid` calls receive the
  current cleanup continuation on their nonreturning failure edge, including
  when evaluating the operand suspended a source call's region. Failure lowering
  does not change the success edge's region state (5.2.7p9, 5.2.8p2, 12.2p3, 15.2p1).
- An out-of-class template member definition retains ABI tags on its earlier
  declaration. The PA29 inspection requires the tagged name, with a specific
  documented Clang-name exception; GCC retains this declaration tag.

PA27's private-name inspection admits both GCC and Clang local discriminator
spellings, as its handout permits. No external linkage requirement is relaxed.

## Comparison allowances and rejection boundaries

PA16 permits omission of unused constexpr support definitions. The comparison
marks reachability from retained entries instead of making one pass over leaves;
optional transitive and cyclic definition graphs can disappear together. A
strong definition, object root, retained body or global reference still keeps
its dependencies required.

The existing exit-finalizer allowance also permits splitting destruction into
registered callbacks. Registration must occur exactly once for each callback;
reverse registration order must execute the same destructor/object/element
sequence as the reference. A newly introduced initializer can contain only those
registrations. Other entries continue to match without inserted effects.

Finalizer comparison now follows normal control flow through a bounded subset
of constant/slot bookkeeping rather than collecting textual calls. It handles
counted array destruction, verifies element counts and order, and rejects a
required destructor hidden behind a false or unknown condition, extra calls,
observable stores, duplicate/missing registrations and unrelated definitions.
Unrecognized finalizer forms do not receive this fallback allowance. Reference
constructed flags are armed for this comparison because conditional callback
registration replaces their guards.

## Regression coverage and policy boundaries

The PA10 operand-order fixture records RHS-before-LHS-address lowering. Eight
PA21 self-checking controls and one PA23 xvalue integration control cover lexical
cleanup, both conditional arms of cast/typeid failure, active catch lifetime,
nested operands, xvalues, pointer casts, successful consumers and throwing
normal retirement. Six fail with the
frozen main compiler and all nine pass with the corrected compiler. The existing
PA21 construction/lifetime controls also remain required.

Harness tests cover split/single registration, constant-initialized objects,
skipped destruction, loop cardinality/order and transitive optional definitions.
Existing strict golden fixtures exercise the remaining producer changes.

The pointer-reference binding in PA10's cv-qualified boundary fixture and the
successful PA19 dependent partial-specialization/SFINAE expectation are unchanged.
Their N3485-versus-later-CWG policy questions are separate from these fixes.

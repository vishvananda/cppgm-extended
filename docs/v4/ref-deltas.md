# Reference deltas (Phase 3 tracker)

Every difference between the references this tree carried before v4 and
the references regenerated from the new implementation is recorded here
once, by class: (a) a LowIR contract change, with the `pa13/lowir.md`
section that states it; (b) a compiler fix, with the fixture that shows
it; (c) a presentation difference the relaxed comparison absorbs.  A delta
that fits no class is a defect to fix.

## Fixtures that leave with the contract

Nine PA13 `tests/spec` fixtures existed only in this tree; the source tree
had removed them when the LowIR contract was minimized
(`docs/implemented/v3/PLAN-LOWIR-CONTRACT-MINIMIZATION.md`; commits
`e87c358c` "remove unused capture and access metadata" and `5a891f80`
"remove decay source provenance" in the source history).  They exercise
metadata the contract no longer has, and six of them fail outright under
the new `lowir2cy86` because that metadata is now rejected:

| fixture | forms | under the new tool |
|---|---|---|
| `200-call-boundary-metadata` | call boundary metadata | rejected |
| `200-index-projection-metadata-smoke` | `index` projection metadata | rejected |
| `200-parameter-access-metadata-smoke` | parameter `access=` | rejected |
| `200-parameter-capture-metadata-smoke` | parameter `capture=` | rejected |
| `200-prototype-relaxed-arity-smoke` | relaxed prototype arity | rejected |
| `200-unary-decay-pointer-smoke` | `unary decay` | rejected |
| `200-bad-nonptr-parameter-access` | `access=` on a non-pointer (negative) | still rejected |
| `200-bad-nonptr-parameter-capture` | `capture=` on a non-pointer (negative) | still rejected |
| `200-bad-unary-decay-nonptr` | `unary decay` on a non-pointer (negative) | still rejected |

Disposition: dropped with the contract (Phase 2).  The three negative
cases pass only because the whole construct is now invalid; keeping them
would document metadata a student must not emit.

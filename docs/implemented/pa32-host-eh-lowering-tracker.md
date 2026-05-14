# PA32 Host EH Lowering Tracker

This tracker records the explicit host-EH review for the current `PA32` owner
surface.

Unlike `PA25`, this tracker compares our `cppgm++ -c` objects directly against
host Clang `-O0` analogues for the same source cases. The useful review layers
here are:

- runtime result
- disassembly around throw/catch/cleanup paths
- normalized host-EH object facts from
  [`dump_host_eh_object_facts.py`](/Users/vishvananda/cppgm/scripts/dump_host_eh_object_facts.py)

## Reviewed Scope

This tranche now covers the currently owned EH owners:

- `191` same-TU throw/catch
- `192` cross-TU throw/catch
- `193` unwind cleanup
- `194` unhandled throw
- `219` typed class-exception base catch
- `220` rethrow
- `221` foreign catch-all
- `222` switch + catch control-flow

## Current Structural Summary

This pass changed the host-EH surface materially:

- the old synthetic helper-table / data-anchor path is gone
- `193` now emits host cleanup metadata on the throwing TU again
- host-EH landing pads now preserve the selector and the current owner surface
  uses that selector instead of `__cxa_current_exception_type`

The remaining differences from Clang `-O0` are the accepted ones for this
assignment boundary:

- macOS objects use compact-unwind plus our local LSDA symbol names rather than
  mirroring Clang's exact `eh_frame` presentation
- some throw-only objects still have ordinary `.data` from emitted globals or
  RTTI, even though the old EH helper-table/data-anchor path is gone

## 191 same-TU throw/catch

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Review note:

- the old helper-table indirection is gone
- typed catch now uses the landing-pad selector instead of
  `__cxa_current_exception_type`

## 192 cross-TU throw/catch

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Throwing TU:

```text
reviewed / fine
```

Review note:

- after removing the old helper-table path, the catch TU now matches the
  Clang helper surface for this owner

## 193 unwind cleanup

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Throwing / cleanup TU, our key facts:

```text
undef _Unwind_Resume
undef __gxx_personality_v0
section gcc_except_table
reloc text branch32 _Unwind_Resume
```

Clang throwing / cleanup TU:

```text
undef _Unwind_Resume
undef __gxx_personality_v0
section eh_frame
section gcc_except_table
reloc text branch32 _Unwind_Resume
```

Review note:

- the earlier missing cleanup metadata on the throwing TU is fixed
- the cleanup observer now uses the selector-driven typed catch path too

## 194 unhandled throw

Status: `reviewed / fine`

Runtime:

```text
program exit status: 0
```

Our key facts:

```text
undef __cxa_allocate_exception
undef __cxa_throw
section compact_unwind
section text
```

Review note:

- this is now a clean throw-only owner
- the residual compact-unwind vs `eh_frame` difference on macOS is not being
  treated as a host-EH lowering bug at this assignment boundary

## 219 typed class-exception base catch

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Review note:

- this owner now takes the same selector-driven path as the scalar typed-catch
  owners; we no longer probe `__cxa_current_exception_type`

## 220 rethrow

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Review note:

- the rethrow path itself is structurally fine
- both the inner and outer typed catches now use the selector path rather than
  an explicit RTTI probe

## 221 foreign catch-all

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Our key facts:

```text
undef __cxa_begin_catch
undef __cxa_end_catch
section gcc_except_table
reloc text branch32 __cxa_begin_catch
reloc text branch32 __cxa_end_catch
```

Clang `-O0` analogue:

```text
undef __cxa_begin_catch
undef __cxa_end_catch
section eh_frame
section gcc_except_table
reloc text branch32 __cxa_begin_catch
reloc text branch32 __cxa_end_catch
```

Review note:

- `catch (...)` no longer calls `__cxa_current_exception_type`
- this owner is now structurally aligned with Clang apart from the accepted
  compact-unwind vs `eh_frame` difference

## 222 switch + catch control-flow

Status: `simplified here`

Runtime:

```text
program exit status: 0
```

Review note:

- control-flow lowering through the `switch`/`break` shape is correct
- the typed catch now uses the selector path instead of
  `__cxa_current_exception_type`

## Current Conclusion

This host-EH review tranche is complete for the current `PA32` owner surface.
The current state is:

- throw-only owners are in good shape
- cleanup/unwind metadata is back on the `193` throwing TU
- `catch (...)` is simplified to the expected host helper surface
- typed catches now use selector-driven dispatch on the reviewed owner set

No further simple worthwhile host-EH lowering fix remains in this current
owner bucket. Future work in this area should come back only if a new `PA32`
owner exposes a structurally different host-EH mismatch.

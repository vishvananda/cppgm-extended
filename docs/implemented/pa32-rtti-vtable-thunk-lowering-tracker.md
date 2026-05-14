# PA32 RTTI / Vtable / Thunk Lowering Tracker

This tracker records the explicit case-by-case review for the PA32 host
RTTI/vtable/thunk surface.

The oracle for each reviewed owner combines:

- runtime behavior
- disassembly shape
- normalized object facts from
  [`scripts/dump_host_abi_object_facts.py`](/Users/vishvananda/cppgm/scripts/dump_host_abi_object_facts.py)

Status meanings:

- `reviewed / fine`
- `simplified here`
- `tracked gap`
- `not meaningfully comparable`

## Initial Ownership Tranche

The first tranche focuses on the smallest ownership-heavy PA32 owners:

1. `118-polymorphic-inline-header-duplicate`
2. `125-inline-ctor-external-vtable-import`
3. `205-host-external-rtti-import`
4. `208-host-imported-covariant-return-adjustment`

## Reviewed Owners

### 118-polymorphic-inline-header-duplicate

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `118-polymorphic-inline-header-duplicate.t.1` and `.t.2`
  compiled with `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
define _ZTI4Poly
define _ZTS4Poly
define _ZTV4Poly
undef _ZTVN10__cxxabiv117__class_type_infoE
section rodata
section text
reloc rodata abs64 _ZN4PolyD0Ev
reloc rodata abs64 _ZN4PolyD1Ev
reloc rodata abs64 _ZNK4Poly1fEv
reloc rodata abs64 _ZTI4Poly
reloc rodata abs64 _ZTS4Poly
reloc rodata abs64 _ZTVN10__cxxabiv117__class_type_infoE
reloc text gotpcrel _ZTV4Poly
```

- `cppgm++` object facts:

```text
object_format mach-o
define _ZTI4Poly
define _ZTS4Poly
define _ZTV4Poly
undef _ZTVN10__cxxabiv117__class_type_infoE
section data
section rodata
section text
reloc rodata abs64 _ZN4PolyD0Ev
reloc rodata abs64 _ZN4PolyD1Ev
reloc rodata abs64 _ZNK4Poly1fEv
reloc rodata abs64 _ZTI4Poly
reloc rodata abs64 _ZTS4Poly
reloc rodata abs64 _ZTVN10__cxxabiv117__class_type_infoE
reloc text pcrel32 _ZTV4Poly
```

- Review note: ownership is correct, and the vtable/RTTI/typeinfo-name payload
  now lives in readonly storage on both Mach-O and ELF. The remaining
  difference is quality rather than ownership: Clang still uses GOT-indirected
  address-point loads where `cppgm++` uses direct PC-relative references.

### 125-inline-ctor-external-vtable-import

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `125-inline-ctor-external-vtable-import.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
undef _ZTV8HostPoly
section text
reloc text gotpcrel _ZTV8HostPoly
```

- `cppgm++` object facts:

```text
object_format mach-o
undef _ZTV8HostPoly
section data
section text
reloc text gotpcrel _ZTV8HostPoly
```

- Disassembly comparison:

```text
clang HostPoly::HostPoly():
  movq    (%rip), %rcx            ## _ZTV8HostPoly@GOTPCREL
  addq    $0x10, %rcx
  movq    %rcx, (%rax)

cppgm HostPoly::HostPoly():
  leaq    (%rip), %r9             ## _ZTV8HostPoly
  addq    $0x10, %rax
  movq    %rax, (%rcx)
```

- Review note: this owner now imports the external vtable group address point
  the same way as Clang on both Mach-O and ELF. The earlier local-definition
  ownership bug is gone; the remaining differences are no longer about vtable
  ownership for this case.

### 205-host-external-rtti-import

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `205-host-external-rtti-import.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
undef _ZTI11HostDerived
undef _ZTI8HostBase
undef __dynamic_cast
section text
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI11HostDerived
reloc text gotpcrel _ZTI8HostBase
```

- `cppgm++` object facts:

```text
object_format mach-o
define _ZTI11HostDerived
define _ZTI8HostBase
define _ZTS11HostDerived
define _ZTS8HostBase
define _ZTV11HostDerived
define _ZTV8HostBase
undef _ZTVN10__cxxabiv117__class_type_infoE
undef _ZTVN10__cxxabiv120__si_class_type_infoE
section data
section rodata
section text
reloc rodata abs64 _ZN11HostDerivedD0Ev
reloc rodata abs64 _ZN11HostDerivedD1Ev
reloc rodata abs64 _ZN8HostBaseD0Ev
reloc rodata abs64 _ZN8HostBaseD1Ev
reloc rodata abs64 _ZNK11HostDerived5valueEv
reloc rodata abs64 _ZTI11HostDerived
reloc rodata abs64 _ZTI8HostBase
reloc rodata abs64 _ZTS11HostDerived
reloc rodata abs64 _ZTS8HostBase
reloc rodata abs64 _ZTVN10__cxxabiv117__class_type_infoE
reloc rodata abs64 _ZTVN10__cxxabiv120__si_class_type_infoE
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI11HostDerived
reloc text gotpcrel _ZTI8HostBase
reloc text pcrel32 _ZTV11HostDerived
```

- Disassembly comparison:

```text
clang:
  movq    (%rip), %rsi            ## _ZTI8HostBase@GOTPCREL
  movq    (%rip), %rdx            ## _ZTI11HostDerived@GOTPCREL
  callq   ___dynamic_cast

cppgm:
  leaq    (%rip), %rbx            ## _ZTV11HostDerived
  addq    $0x10, %rax
  cmpq    %r12, %rax
```

- Review note: this owner now uses the ordinary host `__dynamic_cast` path with
  GOT-loaded `_ZTI*` operands on both Mach-O and ELF, and the locally emitted
  RTTI/vtable payload now sits in readonly storage. The remaining gap is
  external ownership quality rather than cast lowering or storage packaging:
  `cppgm++` still defines the derived/base RTTI and vtables locally instead of
  importing the external RTTI ownership shape Clang uses here.

### 208-host-imported-covariant-return-adjustment

- Status: `reviewed / fine`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `208-host-imported-covariant-return-adjustment.t.1`
  compiled with `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
section text
```

- `cppgm++` object facts:

```text
object_format mach-o
section data
section text
```

- Disassembly comparison:

```text
clang:
  movq    (%rdi), %rax
  callq   *0x10(%rax)

cppgm:
  movq    (%rcx), %r8
  addq    $0x10, %rax
  movq    (%rcx), %r12
  callq   *%r10
```

- Review note: this owner does not expose a distinct RTTI/vtable ownership bug.
  The remaining difference is a more verbose virtual-call path, which is better
  treated as ordinary dispatch/code-shape quality rather than a PA32 ABI
  ownership problem.

## RTTI Review Tranche

This tranche reviews the next STL-free RTTI / MI / VI owners:

1. `210-host-dynamic-cast-ref-failure`
2. `211-host-typeid-null-polymorphic-throw`
3. `212-host-typeid-dynamic-type`
4. `213-host-multiple-inheritance-cross-cast`
5. `214-host-virtual-inheritance-typed-cross-cast`
6. `218-host-self-covariant-return-adjustment`

### 210-host-dynamic-cast-ref-failure

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `210-host-dynamic-cast-ref-failure.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
define _ZTI4Base
define _ZTI4Left
define _ZTI5Right
define _ZTS4Base
define _ZTS4Left
define _ZTS5Right
define _ZTV4Base
define _ZTV4Left
undef _ZTVN10__cxxabiv117__class_type_infoE
undef _ZTVN10__cxxabiv120__si_class_type_infoE
undef __cxa_bad_cast
undef __dynamic_cast
section rodata
section text
reloc text branch32 __cxa_bad_cast
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI4Base
reloc text gotpcrel _ZTI5Right
```

- `cppgm++` object facts:

```text
object_format mach-o
define _ZTI4Base
define _ZTI4Left
define _ZTI5Right
define _ZTS4Base
define _ZTS4Left
define _ZTS5Right
define _ZTV4Base
define _ZTV4Left
define _ZTV5Right
undef _ZTVN10__cxxabiv117__class_type_infoE
undef _ZTVN10__cxxabiv120__si_class_type_infoE
undef __cxa_bad_cast
undef __dynamic_cast
section data
section rodata
section text
reloc rodata abs64 _ZN4BaseD0Ev
reloc rodata abs64 _ZN4BaseD1Ev
reloc rodata abs64 _ZN4LeftD0Ev
reloc rodata abs64 _ZN4LeftD1Ev
reloc rodata abs64 _ZN5RightD0Ev
reloc rodata abs64 _ZN5RightD1Ev
reloc rodata abs64 _ZTI4Base
reloc rodata abs64 _ZTI4Left
reloc rodata abs64 _ZTI5Right
reloc rodata abs64 _ZTS4Base
reloc rodata abs64 _ZTS4Left
reloc rodata abs64 _ZTS5Right
reloc rodata abs64 _ZTVN10__cxxabiv117__class_type_infoE
reloc rodata abs64 _ZTVN10__cxxabiv120__si_class_type_infoE
reloc text branch32 __cxa_bad_cast
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI4Base
reloc text gotpcrel _ZTI5Right
```

- Review note: this owner now takes the host `__dynamic_cast` plus
  `__cxa_bad_cast` path instead of the old local vtable scan, and the extra
  local RTTI/vtable payload now lives in readonly storage. The remaining gap is
  ownership quality: `cppgm++` still materializes extra local RTTI that Clang
  does not need for this path.

### 211-host-typeid-null-polymorphic-throw

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `211-host-typeid-null-polymorphic-throw.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
undef __cxa_bad_typeid
section text
reloc text branch32 __cxa_bad_typeid
```

- `cppgm++` object facts:

```text
object_format mach-o
undef __cxa_bad_typeid
section data
section text
reloc text branch32 __cxa_bad_typeid
```

- Review note: this owner no longer forces local RTTI or vtable emission on
  either Mach-O or ELF. The remaining object-level difference versus Clang is
  just a generic extra `.data` section, not a host RTTI/vtable ownership gap.

### 212-host-typeid-dynamic-type

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `212-host-typeid-dynamic-type.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Review note: the lowering now takes the host vptr-to-typeinfo path instead of
  scanning local vtable candidates. The remaining difference is quality rather
  than mechanism: the RTTI/vtable surface is now readonly like Clang's, but
  `cppgm++` still uses direct PC-relative references where Clang prefers
  GOT-indirected loads.

### 213-host-multiple-inheritance-cross-cast

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `213-host-multiple-inheritance-cross-cast.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
define _ZTI7Derived
define _ZTI8LeftBase
define _ZTI9RightBase
define _ZTS7Derived
define _ZTS8LeftBase
define _ZTS9RightBase
define _ZTV7Derived
define _ZTV8LeftBase
define _ZTV9RightBase
define _ZThn8_N7DerivedD0Ev
define _ZThn8_N7DerivedD1Ev
undef _ZTVN10__cxxabiv117__class_type_infoE
undef _ZTVN10__cxxabiv121__vmi_class_type_infoE
undef __dynamic_cast
section rodata
section text
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI8LeftBase
reloc text gotpcrel _ZTI9RightBase
```

- `cppgm++` object facts:

```text
object_format mach-o
define _ZTI7Derived
define _ZTI8LeftBase
define _ZTI9RightBase
define _ZTS7Derived
define _ZTS8LeftBase
define _ZTS9RightBase
define _ZTV7Derived
define _ZTV8LeftBase
define _ZTV9RightBase
define _ZThn8_N7DerivedD0Ev
define _ZThn8_N7DerivedD1Ev
define _ZThn8_N7DerivedD2Ev
undef _ZTVN10__cxxabiv117__class_type_infoE
undef _ZTVN10__cxxabiv121__vmi_class_type_infoE
undef __dynamic_cast
section data
section rodata
section text
reloc rodata abs64 _ZN7DerivedD0Ev
reloc rodata abs64 _ZN7DerivedD1Ev
reloc rodata abs64 _ZN8LeftBaseD0Ev
reloc rodata abs64 _ZN8LeftBaseD1Ev
reloc rodata abs64 _ZN9RightBaseD0Ev
reloc rodata abs64 _ZN9RightBaseD1Ev
reloc rodata abs64 _ZTI7Derived
reloc rodata abs64 _ZTI8LeftBase
reloc rodata abs64 _ZTI9RightBase
reloc rodata abs64 _ZTS7Derived
reloc rodata abs64 _ZTS8LeftBase
reloc rodata abs64 _ZTS9RightBase
reloc rodata abs64 _ZTVN10__cxxabiv117__class_type_infoE
reloc rodata abs64 _ZTVN10__cxxabiv121__vmi_class_type_infoE
reloc rodata abs64 _ZThn8_N7DerivedD0Ev
reloc rodata abs64 _ZThn8_N7DerivedD1Ev
reloc text branch32 __dynamic_cast
reloc text gotpcrel _ZTI8LeftBase
reloc text gotpcrel _ZTI9RightBase
reloc text pcrel32 _ZTV7Derived
reloc text pcrel32 _ZTV8LeftBase
reloc text pcrel32 _ZTV9RightBase
```

- Disassembly comparison:

```text
clang:
  movq    _ZTI8LeftBase@GOTPCREL(%rip), %rsi
  movq    _ZTI9RightBase@GOTPCREL(%rip), %rdx
  movq    $-2, %rcx
  callq   ___dynamic_cast

cppgm:
  movq    _ZTI8LeftBase@GOTPCREL(%rip), %r12
  movq    _ZTI9RightBase@GOTPCREL(%rip), %r13
  movabsq $-2, %rcx
  callq   ___dynamic_cast
```

- Review note: this owner was the main worthwhile fix in the tranche. `cppgm++`
  now emits non-single-inheritance RTTI with `__vmi_class_type_info` and uses
  the correct `-2` cross-cast hint, so the real host `__dynamic_cast` path now
  works on both Mach-O and ELF. The RTTI/vtable payload is now in readonly
  storage and the non-virtual destructor thunk surface matches Clang. The
  remaining differences are quality-level ones like direct PC-relative
  address-point references instead of GOT-indirected loads.

### 214-host-virtual-inheritance-typed-cross-cast

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `214-host-virtual-inheritance-typed-cross-cast.t.1`
  compiled with `clang++ -std=gnu++11 -x c++ -c`
- Review note: this owner now uses the host `__dynamic_cast` path, imports the
  source/target RTTI the same way as Clang, exports the non-virtual `Leaf`
  `-8` destructor thunks plus the virtual-base `ZTv0_n24...` thunk family, and
  now also surfaces the same public construction-data symbols:
  `_ZTC4Leaf0_4Left`, `_ZTC4Leaf8_5Right`, `_ZTT4Leaf`, `_ZTT4Left`, and
  `_ZTT5Right`. `LeafC1`, `LeafD1`, and `LeafD2` now use `__ZTT4Leaf` slices,
  and `LeftC2` / `RightC2` consume VTT entries on both Mach-O and ELF. The
  remaining differences are quality-level ones: Clang still uses a richer VTT
  / construction-vtable packaging shape with extra `__ZTT4Left` /
  `__ZTT5Right` references and more GOT-indirected address-point loads, while
  `cppgm++` uses a narrower minimal VTT slice model.

### 218-host-self-covariant-return-adjustment

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `218-host-self-covariant-return-adjustment.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Review note: this owner now exports the real Itanium covariant-return thunk
  `_ZTchn8_h8_N7Derived4selfEv` plus the matching non-virtual destructor thunks
  on both Mach-O and ELF. The remaining differences are no longer about missing
  host-visible thunk entrypoints for this owner; the broader virtual-inheritance
  / VTT / construction-vtable surface remains tracked separately under `214`.

## Destructor Dispatch Review Tranche

This tranche reviews the remaining destructor/phase owners:

1. `215-host-delete-through-base-dtor`
2. `216-host-constructor-virtual-dispatch`
3. `217-host-destructor-virtual-dispatch`

### 215-host-delete-through-base-dtor

- Status: `simplified here`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `215-host-delete-through-base-dtor.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Clang object facts:

```text
object_format mach-o
define _ZN4BaseD0Ev
define _ZN4BaseD1Ev
define _ZN4BaseD2Ev
define _ZN7DerivedD0Ev
define _ZN7DerivedD1Ev
define _ZN7DerivedD2Ev
define _ZTV4Base
define _ZTV7Derived
section rodata
section text
reloc text gotpcrel _ZTV4Base
reloc text gotpcrel _ZTV7Derived
```

- `cppgm++` object facts:

```text
object_format mach-o
define _ZN4BaseD0Ev
define _ZN4BaseD1Ev
define _ZN4BaseD2Ev
define _ZN7DerivedD0Ev
define _ZN7DerivedD1Ev
define _ZN7DerivedD2Ev
define _ZTV4Base
define _ZTV7Derived
section data
section rodata
section text
reloc text pcrel32 _ZTV4Base
reloc text pcrel32 _ZTV7Derived
```

- Disassembly comparison:

```text
clang:
  movq    (%rax), %rax
  callq   *0x8(%rax)

cppgm:
  movq    (%rcx), %r8
  addq    $0x8, %rax
  movq    (%rcx), %r12
  callq   *%r10
```

- Review note: this owner was the next real ABI fix. `cppgm++` now emits true
  deleting destructors (`D0`) and dispatches `delete` through the deleting
  destructor slot instead of calling a complete destructor plus a separate
  `operator delete`. The remaining difference is codegen quality rather than
  ABI surface: the vtables are now in readonly data, but `cppgm++` still uses
  direct PC-relative references where Clang loads the address point through
  GOT-indirected access.

### 216-host-constructor-virtual-dispatch

- Status: `reviewed / fine`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `216-host-constructor-virtual-dispatch.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Review note: after the deleting-destructor fix, this owner no longer exposes
  a distinct constructor-phase dispatch bug. The remaining differences are the
  same readonly-vtable ownership and address-point materialization quality gaps
  already tracked elsewhere.

### 217-host-destructor-virtual-dispatch

- Status: `reviewed / fine`
- Runtime: existing owner passes on host and Linux.
- Clang analogue: `217-host-destructor-virtual-dispatch.t.1` compiled with
  `clang++ -std=gnu++11 -x c++ -c`
- Review note: destructor-phase virtual dispatch is behaving correctly for this
  owner on both Mach-O and ELF. As with `216`, the remaining visible delta is
  storage/relocation quality rather than another missing thunk or dispatch
  rule.

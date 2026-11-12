# Shared machine boundary review

This maintainer review accompanies PA37 and is excluded from the student export.
It checks the boundary against an AArch64 register target without implementing
or advertising an AArch64 compiler target.

The selected instruction streams remain target-specific: `native::Function`
contains x86 operands and its physical ABI pieces; `wasm::Function` contains
Wasm typed locals and operand ranges. Both encoders and dumpers consume their
respective selected streams. Neither backend maintains a second instruction
stream solely for generic analyses.

The reusable algorithms consume typed LowIR or explicitly supplied graph views.
`machine/call_arguments.cpp` retains formal types and unnamed-argument shape/type
facts before target classification. `machine/exception_regions.cpp` and
`machine/exception_tables.cpp` consume the caller's CFG and source-level clauses,
without choosing frame offsets or physical exception registers.
`machine/global_data.cpp` lays out target-sized constants and address fixups;
object writers select relocation encodings. Native register allocation and Wasm
local cleanup remain separate because their placement problems differ.

| AArch64 pressure | Boundary and evidence |
| --- | --- |
| Integer/vector banks and overlapping W/X views | Register IDs in `machine_model::Location` are 32-bit target identities. Alias units and register banks belong to the register target's description. The representation probe uses distinct W0/X0 IDs sharing one alias unit, and a vector ID above 31; no common mask imposes x86's register count. |
| Three-address operations and variable arity | Shared `Range` describes operand slices; Wasm emission already consumes it. The probe represents a distinct destination plus two inputs and a 17-operand call. The native instruction's three inline operands remain its own encoding choice. |
| Four-part homogeneous aggregate results | LowIR `AggregateShape` retains element kind and count independently of AMD64 classes. The probe passes a four-float aggregate through the actual shared call-fact reader, then represents four vector locations and a distinct indirect result register. The two-element native result array is not exposed by a common analysis. |
| SP, frame pointer and link register | Shared frame objects retain size, alignment and identity; a location distinguishes frame objects, incoming stack, registers and Wasm locals. The probe keeps aligned storage separate from signed frame offsets. Prologues, register saves and unwind encodings remain target policies. |
| Immediate/addressing ranges, branches and relocations | Selected opcodes, operand legality, layout/relaxation and relocations are target payloads. Shared global fixups carry symbol identity and byte addends, not x86 addressing forms. Wasm has its own encoder and relocation writer. |
| Linux versus Darwin ABI | Architecture, environment, object format, calling convention and data layout are distinct fields. A future AArch64 implementation must add its calling-convention identities and descriptions, rather than reuse an AMD64 ABI merely because the OS matches. No current CLI target claims AArch64 support. |

`scripts/machine_boundary_contract.cpp` is compiled and executed by
`scripts/tests/test_machine_boundary.py` in the harness. It exercises the
shared call-fact reader without linking an x86 or Wasm selector. Its register
locations are a representation probe, not an implementation or qualification
of AAPCS64 classification, register allocation or instruction encoding.

Portable aggregate and vector shapes still describe the course's current
fixed-width types. Scalable vectors, big-endian code generation and a third
encoder remain future extensions; the review does not claim they work.

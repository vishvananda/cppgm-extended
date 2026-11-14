# CPPGM Assignment Roadmap

This is the canonical assignment order for the active assignment buildout.
Detailed contracts live in each `paN/README.md`; implementation is cumulative
under `dev/` and `dev/src/`.

| Range | Milestones |
| --- | --- |
| PA1-PA4 | tokens, literals, preprocessing expressions, and full preprocessing |
| PA5-PA7 | AST, types/lookup, conversions, calls, and overload resolution |
| PA8 | LowIR model, reader/writer, and required construction exercises (`lowir`) |
| PA9 | typed Itanium ABI name construction (`abimangle`) |
| PA10-PA13 | procedural LowIR lowering, classes, value semantics, and virtual dispatch |
| PA14-PA19 | templates, metaprogramming, constant evaluation, completion, and integration |
| PA20-PA23 | language and object-model closure through virtual inheritance and RTTI |
| PA24 | LowIR-to-native backend (`lowir2native`) |
| PA25 | separate compilation and the `cppgm++` compile/link driver |
| PA26 | host exception metadata and runtime-helper facts |
| PA27-PA28 | host-linkable objects and host C++ ABI/runtime interoperation |
| PA29 | shared assembly constraints, integrated encoding and object behavior |
| PA30-PA32 | hosted source/header compatibility and hosted link/runtime behavior |
| PA33-PA34 | LowIR and machine/backend optimization |
| PA35 | staged self-host ladder and inception |
| PA36 | optimizing the compiler: generated code against its own host-built seed and GCC, compile speed against the host |
| PA37 | wasm32 backend, reusable machine representations, target layout, ABI and module interoperability |
| PA38 (optional) | C frontend using the existing typed LowIR boundary |

## PA9 ABI Boundary

PA9 owns the typed ABI model and Itanium encoder before the compiler first
emits symbols in PA10. The standalone `abimangle` tool adapts normalized fact
files into that model. Compiler stages construct typed targets and call the
same encoder directly; the fact text format is not an internal compiler
transport.

## Required language and assembly profile

The [required language profile](LANGUAGE_PROFILE.md) places the supported
C++14/17 forms in their core lessons. The shared LowIR assembly transport is
introduced in PA8; integrated assembly follows PA28 host ABI work and precedes
hosted compatibility. Normal reports include these required checks.

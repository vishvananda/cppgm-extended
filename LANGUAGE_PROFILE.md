# Required source-language profile

Build a C++ compiler with the following C++11/14/17 profile. This is a bounded
course contract, not complete conformance to all three standards. Later hosted
lessons consume these features rather than making them optional.

The driver default remains GNU C++11. `-std=c++11`, `-std=c++14`, `-std=c++17`
and their `gnu++` counterparts select the documented mode. A required later
feature need only work in its stated mode; earlier-mode negative checks remain
required. The existing variable-template, relaxed constexpr and visible-body
return-deduction extensions remain available in the course's C++11 mode.

| Feature | Mode | Owning lessons |
| --- | --- | --- |
| Binary literals, digit separators | C++14+ | PA1–PA4 token/value/macro boundaries |
| Hexadecimal floating literals | C++17 | PA1–PA4; GNU compiler modes also retain existing extensions |
| Nested namespaces, attribute using-prefixes | C++17 | PA5 syntax, PA6 lookup; vendor effects PA29 |
| Contextual conversions, aggregate member defaults | C++14+ | PA11/PA12; constexpr PA16 |
| Public aggregate bases, direct prvalue result objects, inline variables, aligned allocation | C++17 | PA11/PA12; constexpr PA16; multi-TU PA25; host ABI PA31 |
| Fold expressions | C++17 | PA15; composed deduction PA18/PA19 |
| Relaxed constexpr bodies | C++14 rules, existing C++11 extension | PA16 |
| Variable templates | C++14, existing C++11 extension | PA17 |
| Template-template `typename`, placeholder non-type parameters, class deduction/guides, using packs | C++17 | PA17 declarations, PA18 deduction; multiple-base/member-pointer combinations PA22 |
| Selection initializer statements | C++17 | PA10 |
| `if constexpr` | C++17 | PA19, after substitution and constant evaluation |
| `decltype(auto)`, generic lambdas, initialized captures | C++14+ | PA20; failure cleanup PA21 |
| `[*this]`, constexpr closures, structured bindings, fixed-enum direct lists | C++17 | PA20; failure cleanup PA21 |
| `noexcept` function-type identity/conversion | C++17 | PA6/PA7; ABI PA9; deduction PA18; host checks PA31 |
| General GNU source assembly and shared LowIR assembly | GNU extension | PA8 transport; PA36 binding, integrated encoding and object behavior |
| Standard-library consumers (`optional`, `variant`, `string_view`, deduction guides) | Required selected modes | PA30/PA31; use host headers, no STL implementation required |

Read the owning handout for exact supported forms and limits. Syntax appears
in the shared grammar at PA5; its presence does not require all later semantics
at PA5. Tests select modes with `NAME.cxx-standard` or driver `.flags` metadata
and run in ordinary assignment/cumulative suites.

The [offline references](doc/REFERENCES.md) retain edition-specific wording and
adopted-defect provenance. Selected later header extensions, including
conditional `explicit`, alias initializers and `no_unique_address`, remain
separately documented compatibility concessions, not full C++20/23 support.

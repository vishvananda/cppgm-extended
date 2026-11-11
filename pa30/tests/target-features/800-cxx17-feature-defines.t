#if __cpp_enumerator_attributes < 201411L || __cpp_namespace_attributes < 201411L || __cpp_hex_float < 201603L
#error C++17 attributes and hexadecimal floating literals must be advertised
#endif
namespace [[deprecated]] old_namespace { enum Value { old_value [[deprecated]] = 4 }; }
static_assert(0x1.8p2 == 6.0);
#if __cpp_if_constexpr < 201606L || __cpp_inline_variables < 201606L || __cpp_fold_expressions < 201603L
#error C++17 compile-time and inline features must be advertised
#endif
#if __cpp_structured_bindings < 201606L || __cpp_noexcept_function_type < 201510L || __cpp_constexpr < 201603L
#error C++17 type and constexpr features must be advertised
#endif
#if __cpp_aggregate_bases < 201603L || __cpp_nontype_template_parameter_auto < 201606L
#error C++17 aggregate and template features must be advertised
#endif
inline int state = 4;
template<auto N, class... T> constexpr int sum(T... values) { return (N + ... + values); }
template<class T> constexpr int select(T value) { if constexpr (sizeof(T) == sizeof(int)) return value; else return T::missing; }
struct Pair { int first; int second; };
int main() { Pair p{state, 7}; auto [a, b] = p; return sum<1>(a, b) != 12 || select(9) != 9; }

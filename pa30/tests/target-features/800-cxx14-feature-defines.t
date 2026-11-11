#if __cpp_constexpr < 201304L || __cpp_generic_lambdas < 201304L || __cpp_init_captures < 201304L
#error C++14 language features must be advertised
#endif
#if __cpp_decltype_auto < 201304L || __cpp_return_type_deduction < 201304L || __cpp_sized_deallocation < 201309L
#error C++14 deduction and allocation features must be advertised
#endif
#if defined(__cpp_if_constexpr) || defined(__cpp_inline_variables)
#error C++17 feature macros must follow the selected standard
#endif
constexpr int sum(int n) { int result = 0; for (int i = 0; i != n; ++i) result += i; return result; }
static_assert(sum(4) == 6, "relaxed constexpr");
int main() { auto add = [offset = 3](auto value) { return offset + value; }; return add(4) != 7; }

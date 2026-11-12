namespace std {
template<class T> T* addressof(T& value) noexcept { return __builtin_addressof(value); }
}
void function() {}
constexpr auto pointer = std::addressof(function);
static_assert(pointer == &function, "Clang-compatible C++11 std::addressof");

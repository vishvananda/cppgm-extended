namespace ordinary {
template<class T> T* addressof(T& value) noexcept { return __builtin_addressof(value); }
}
void function() {}
constexpr auto pointer = ordinary::addressof(function);

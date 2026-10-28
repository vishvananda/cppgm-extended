template<class T> constexpr T read(T value) {
    return reinterpret_cast<const T&>(value);
}
// Folding the caller's selected expression does not relax the callee's body.
constexpr int value = __builtin_constant_p(0) ? read(7) : read(7);

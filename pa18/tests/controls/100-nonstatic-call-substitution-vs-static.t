// expect exit 0
// N3485 focus: 14.8.2 [temp.deduct], 9.3.1 [class.mfct.non-static].
// Calling a member without an object fails an immediate substitution,
// including an unevaluated operand. Static member calls remain valid.
struct Instance { int value() const { return 3; } };
struct Static { static constexpr int value() { return 3; } };
template<class T, int = T::value()> int by_value(T*) { return 1; }
int by_value(...) { return 0; }
template<class T> auto by_type(T*) -> decltype(T::value(), int()) { return 1; }
int by_type(...) { return 0; }
int main() {
    return by_value((Instance*)0) != 0 || by_type((Instance*)0) != 0 ||
           by_value((Static*)0) != 1 || by_type((Static*)0) != 1;
}

// N3485 7.1.5p9, 5.19p2: scalar initialization consumes recorded constants
// even through member calls and functional casts; runtime effects remain.
struct Bounds {
    static constexpr long low() { return -9223372036854775807L - 1; }
    static constexpr long high() { return 9223372036854775807L; }
};
struct Property { constexpr int value() const { return 42; } };
int calls;
Property side_object() { ++calls; return Property(); }
int runtime_value() { ++calls; return 17; }
long observe(const long* value) { return *value; }

long direct() {
    const long value = long(Bounds::low());
    return value;
}
bool addressed() {
    const long low = long(Bounds::low());
    const long high = static_cast<long>(Bounds::high());
    const volatile long stored = long(Bounds::high());
    return observe(&low) == Bounds::low() && observe(&high) == Bounds::high()
        && stored == high;
}
int effects() {
    const int first = side_object().value();
    const int second = int(runtime_value());
    return first + second;
}
int main() {
    return direct() == Bounds::low() && addressed() && effects() == 59 && calls == 2 ? 0 : 1;
}

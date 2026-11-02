// GNU may_alias permits reads through an otherwise incompatible glvalue.
typedef unsigned __attribute__((may_alias)) Any;
struct __attribute__((may_alias)) AliasCell { unsigned bits; };
union Bits { float f; unsigned u; };
__attribute__((noinline)) float change(float* f, Any* bits) {
    float before = *f; *bits = 0x40000000u; return before + *f;
}
__attribute__((noinline)) float change_class(float* f, AliasCell* cell) {
    float before = *f; cell->bits = 0x40800000u; return before + *f;
}
int main() {
    volatile float one = 1;
    Bits data; data.f = one;
    if (change(&data.f, reinterpret_cast<Any*>(&data.u)) != 3) return 1;
    data.f = one;
    if (change_class(&data.f, reinterpret_cast<AliasCell*>(&data.u)) != 5) return 2;
    return 0;
}

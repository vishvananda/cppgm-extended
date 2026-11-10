#include <cstdio>
#include <type_traits>
// N3485 12.8p15,p28: implicit class copies access volatile subobjects.
struct Empty {};
struct Scalar { volatile int value; };
struct Array { volatile int value[10]; };
struct Nested { Scalar scalar; Scalar values[2]; };
struct Mixed { int first; Empty empty; volatile int last; };
struct Base { volatile int value; char tag; ~Base() {} };
struct Derived : Base { char sentinel; };
union Union { volatile int value; long other; };
struct Wrapped { int first; Union value; };
struct Pointer { volatile int* value; };
struct VolatilePointer { int* volatile value; };
struct Bits { volatile unsigned value : 3; unsigned rest : 5; };
Scalar scalar_copy(const Scalar& s) { return Scalar(s); }
Scalar scalar_move(Scalar& s) { return Scalar(static_cast<Scalar&&>(s)); }
void scalar_assign(Scalar& d, const Scalar& s) { d = s; }
void scalar_move_assign(Scalar& d, Scalar& s) { d = static_cast<Scalar&&>(s); }
void array_assign(Array& d, const Array& s) { d = s; }
void nested_assign(Nested& d, const Nested& s) { d = s; }
void mixed_assign(Mixed& d, const Mixed& s) { d = s; }
void union_assign(Union& d, const Union& s) { d = s; }
void wrapped_assign(Wrapped& d, const Wrapped& s) { d = s; }
void pointer_assign(Pointer& d, const Pointer& s) { d = s; }
void volatile_pointer_assign(VolatilePointer& d, const VolatilePointer& s) { d = s; }
void bits_assign(Bits& d, const Bits& s) { d = s; }
static_assert(std::is_trivially_copy_constructible<Scalar>::value, "volatile does not change copy triviality");
static_assert(std::is_trivially_copy_assignable<Scalar>::value, "volatile does not change assignment triviality");
int run() {
    Scalar a = {17}, b = {0};
    scalar_assign(b, a);
    Scalar c = scalar_copy(b), d = scalar_move(c);
    scalar_move_assign(b, d);
    int bad = b.value != 17;
    Array first = {{1,2,3,4,5,6,7,8,9,10}}, second = {};
    Array copied(first);
    array_assign(second, copied);
    for (int i = 0; i != 10; ++i) bad += second.value[i] != i + 1;
    Nested n = {{23}, {{29},{31}}}, m = {};
    nested_assign(m, n);
    bad += m.scalar.value != 23 || m.values[0].value != 29 || m.values[1].value != 31;
    Mixed x = {5, {}, 37}, y = {};
    mixed_assign(y, x);
    bad += y.first != 5 || y.last != 37;
    Derived derived; derived.value = 41; derived.tag = 7; derived.sentinel = 99;
    Base source; source.value = 43; source.tag = 11;
    static_cast<Base&>(derived) = source;
    bad += derived.value != 43 || derived.tag != 11 || derived.sentinel != 99;
    Union u = {47}, v = {};
    Union union_copy(u);
    union_assign(v, union_copy);
    bad += v.value != 47;
    Wrapped w = {13, {53}}, z = {};
    wrapped_assign(z, w);
    bad += z.first != 13 || z.value.value != 53;
    int value = 59;
    Pointer p = {&value}, q = {};
    pointer_assign(q, p);
    VolatilePointer vp = {&value}, vq = {};
    volatile_pointer_assign(vq, vp);
    bad += q.value != &value || vq.value != &value;
    Bits bits = {6, 19}, bits_copy = {};
    bits_assign(bits_copy, bits);
    bad += bits_copy.value != 6 || bits_copy.rest != 19;
    return bad != 0;
}
int main() { int bad = run(); std::printf("%d\n", bad); return bad; }

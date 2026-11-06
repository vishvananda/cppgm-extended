// Constant initialization preserves shared bytes, signed fields and zeros.
// expect exit 0. CXX11-029 cumulative compiler performance followup.
struct Shared { unsigned long long low : 37; unsigned long long high : 27; int ordinary; };
constexpr Shared shared = {0x1fffffffffULL, 0x7ffffffULL, 19};
struct Signed { unsigned char lead : 3; signed int value : 13; unsigned int tail : 7; };
constexpr Signed signed_fields = {5, -17, 65};
struct Zeros { unsigned long long low : 37; unsigned long long high : 27; };
constexpr Zeros zeros = {0, 0};
static_assert(shared.low == 0x1fffffffffULL && shared.high == 0x7ffffffULL, "shared unit");
static_assert(signed_fields.value == -17 && signed_fields.tail == 65, "signed chunks");

int main()
{
    const volatile Shared* s = &shared;
    const volatile Signed* n = &signed_fields;
    const volatile Zeros* z = &zeros;
    return s->low != 0x1fffffffffULL || s->high != 0x7ffffffULL ||
        s->ordinary != 19 || n->lead != 5 || n->value != -17 ||
        n->tail != 65 || z->low != 0 || z->high != 0;
}

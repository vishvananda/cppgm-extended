// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<int... N> struct alignas(N...) alignas(32) A { char value; };
static_assert(alignof(A<>) == 32, "empty keeps following alignment");
static_assert(alignof(A<8,16>) == 32, "following maximum");
int main() { return sizeof(A<>) != 32; }

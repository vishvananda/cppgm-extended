// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<int... N> struct alignas(N...) A { char value; };
static_assert(alignof(A<>) == 1, "empty values");
int main() { return sizeof(A<>) != 1; }

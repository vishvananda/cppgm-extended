// N3485 8.5.1p7, 5.19: a zero aggregate tail remains compact when
// only its end leaves are read. Shared types are checked once. expect exit 0
struct T0 { int value; };
struct T1 { T0 left, right; };
struct T2 { T1 left, right; };
struct T3 { T2 left, right; };
struct T4 { T3 left, right; };
struct T5 { T4 left, right; };
struct T6 { T5 left, right; };
struct T7 { T6 left, right; };
struct T8 { T7 left, right; };
struct T9 { T8 left, right; };
struct T10 { T9 left, right; };
struct T11 { T10 left, right; };
struct T12 { T11 left, right; };
struct T13 { T12 left, right; };
struct T14 { T13 left, right; };
struct T15 { T14 left, right; };
struct T16 { T15 left, right; };
struct T17 { T16 left, right; };
struct T18 { T17 left, right; };
const T18 (&values)[2] = {};
static_assert(sizeof(values) == 2097152ULL, "full aggregate span");
static_assert(values[0].left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.value == 0 && values[1].right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.value == 0, "zero leaves");
int main()
{
    const volatile int* first = &values[0].left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.value;
    const volatile int* last = &values[1].right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.value;
    return *first != 0 || *last != 0;
}

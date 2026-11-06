// N3485 5.19, 8.5.1: omitted class subobjects retain zero values and distinct identities. expect exit 0
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
constexpr T18 value = {};
static_assert(sizeof(value)==1048576ULL,"full object span");
static_assert(value.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.value==0 && value.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.value==0,"end leaves");
static_assert(&value.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.value != &value.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.value,"distinct places");
int main(){const volatile T18* p=&value;return p->left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.left.value!=0 || p->right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.right.value!=0;}

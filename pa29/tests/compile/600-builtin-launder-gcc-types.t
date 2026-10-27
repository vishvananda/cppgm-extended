// GCC permits incomplete object pointers and removes top-level pointer cv.
int array[3];
int* const fixed = array;
static_assert(__is_same(decltype(__builtin_launder(fixed)), int*), "pointer value type");
struct Incomplete;
Incomplete* incomplete(Incomplete* value) { return __builtin_launder(value); }
int main() {}

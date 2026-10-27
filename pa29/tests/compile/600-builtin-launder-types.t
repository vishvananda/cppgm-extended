#if !__has_builtin(__builtin_launder)
#error builtin registry must expose launder
#endif
constexpr int constant = 7;
constexpr const int* pointer = __builtin_launder(&constant);
static_assert(*pointer == 7, "constant pointer");
int array[3];
static_assert(__is_same(decltype(__builtin_launder(array)), int*), "array decay");
static_assert(__is_same(decltype(__builtin_launder(&array)), int (*)[3]), "array object pointer");
const volatile int* qualified;
static_assert(__is_same(decltype(__builtin_launder(qualified)), const volatile int*), "pointee qualifiers");
int* may_throw();
static_assert(noexcept(__builtin_launder(static_cast<int*>(nullptr))), "builtin does not throw");
static_assert(!noexcept(__builtin_launder(may_throw())), "operand can throw");
template<class T, class U>
T cast(U* value) { return __builtin_launder(reinterpret_cast<T>(value)); }
int main() { return cast<const int*>(&constant) != pointer; }

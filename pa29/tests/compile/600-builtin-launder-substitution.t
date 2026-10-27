// Invalid operands fail immediate substitution, as with other typed builtins.
// Clang agrees; GCC diagnoses invalid builtin arguments during substitution.
template<class T>
auto probe(T value) -> decltype(__builtin_launder(value), char{});
long probe(...);
static_assert(sizeof(probe(static_cast<int*>(nullptr))) == sizeof(char), "pointer candidate");
static_assert(sizeof(probe(static_cast<void*>(nullptr))) == sizeof(long), "void pointer substitution");
static_assert(sizeof(probe(static_cast<void(*)()>(nullptr))) == sizeof(long), "function pointer substitution");
static_assert(sizeof(probe(0)) == sizeof(long), "nonpointer substitution");
int main() {}

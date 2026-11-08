// expect exit 0
// Reduced GCC fntmpdefarg8.C (GPL-3.0-or-later): invalid type in default argument.
template<class T, class U = T[sizeof(T) - 5]> char pick(T);
long pick(...);
static_assert(sizeof(pick(1)) == sizeof(long), "unsigned wrap is an invalid array bound");
static_assert(sizeof(pick(1L)) == 1, "valid default array type");
int main() { return 0; }

#ifdef __cplusplus
#error C source must not announce C++
#endif
#if defined(__GNUG__) || defined(__cpp_constexpr) || defined(__EXCEPTIONS)
#error C source inherited C++ feature macros
#endif
#if __STDC_VERSION__ != 201710L || __STDC__ != 1
#error wrong C language version
#endif
#if defined(__has_feature)
#if __has_feature(cxx_exceptions) || __has_feature(cxx_rtti) || __has_feature(cxx_constexpr)
#error C source inherited C++ feature probes
#endif
#endif
#define and 7
#define JOIN(a, b) a##b
int class = 4, bool = 5, new = 6, static_assert = 8;
_Static_assert(sizeof('a') == sizeof(int), "C character literals have type int");
_Static_assert(sizeof(1 < 2) == sizeof(int), "C comparisons have type int");
_Static_assert(sizeof(!0) == sizeof(int), "C logical operators have type int");
static _Bool decrement(_Bool value) { return --value; }
static int truth(int a, int b) { return sizeof(a < b) * 100 + sizeof(a && b) * 10 + sizeof(!a); }
int main(void) {
  int n = 17; void *raw = &n; int *p = raw; char *text = "abc";
  enum Flag { no, yes }; enum Flag flag = 1;
  _Bool active = *p;
  volatile int counter = 1;
  int assigned = (counter = 9), incremented = ++counter, old = counter++;
  int effects = 0;
  int first = 0 && ++effects, second = 1 || ++effects;
  return !(truth(n, 2) == 444 && sizeof(0 || 1) == sizeof(int) &&
           sizeof(text[0]) == 1 && active && decrement(0) == 1 && decrement(1) == 0 && flag == yes && text[1] == 'b' &&
           class + bool + new + static_assert + JOIN(a, nd) == 30 &&
           assigned == 9 && incremented == 10 && old == 10 && counter == 11 &&
           first == 0 && second == 1 && effects == 0);
}

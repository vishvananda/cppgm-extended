// expect exit 0
// N3485 [dcl.init.list], [except.ctor]: every constructed backing-array
// element is destroyed in reverse order before an outer handler runs.
// Exercise small and counted cleanups, with and without a nonmatching catch.
namespace std {
template<class E> class initializer_list {
  const E* first;
  unsigned long count;
  initializer_list(const E* p, unsigned long n) : first(p), count(n) {}
public:
  initializer_list() : first(0), count(0) {}
  unsigned long size() const { return count; }
};
}
int alive, attempts, limit, bad, stack[128];
void checkpoint(int value)
{
  if (++attempts == limit) throw limit;
  stack[alive++] = value;
}
struct S {
  int value;
  S(int v) : value(v) { checkpoint(value); }
  S(const S& s) : value(s.value) { checkpoint(value); }
  ~S() noexcept { if (!alive || stack[--alive] != value) bad = 1; }
};
using IL = std::initializer_list<S>;
unsigned long borrow(const IL& list) { return list.size(); }
unsigned long short_list() { return borrow({{1}, {2}, {3}}); }
unsigned long long_list()
{
  return borrow({{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10},
                 {11}, {12}, {13}, {14}, {15}, {16}, {17}, {18}, {19}, {20}});
}
unsigned long short_in_try()
{
  try { return borrow({{1}, {2}, {3}}); }
  catch (double) { return 1000; }
}
unsigned long long_in_try()
{
  try {
    return borrow({{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10},
                   {11}, {12}, {13}, {14}, {15}, {16}, {17}, {18}, {19}, {20}});
  } catch (double) { return 1000; }
}
int main()
{
  unsigned long (*const probes[])() = {
      short_list, long_list, short_in_try, long_in_try};
  const int sizes[] = {3, 20, 3, 20};
  for (int k = 0; k < 4; ++k) {
    int bound = 0;
    for (limit = 0; limit <= bound + 1; ++limit) {
      alive = attempts = bad = 0;
      int caught = 0;
      unsigned long result = 0;
      try { result = probes[k](); }
      catch (int e) { caught = e; }
      if (limit == 0) bound = attempts;
      if (alive || bad) return 1;
      if (limit > 0 && limit <= bound) {
        if (caught != limit) return 2;
      } else if (caught || result != sizes[k]) return 3;
    }
  }
  return 0;
}

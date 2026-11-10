// Check branch and materialized predicates across signedness and access widths.
typedef unsigned long long U;
volatile unsigned hits, misses;
U digest = 1469598103934665603ULL;
void record(U value) { digest = (digest ^ value) * 1099511628211ULL; }
template<class T> struct Cell { char prefix[13]; T value; char suffix[9]; };
template<unsigned Op, class T>
__attribute__((noinline)) bool predicate(const Cell<T>* p, T rhs)
{
  return Op == 0 ? p->value == rhs : Op == 1 ? p->value != rhs :
         Op == 2 ? p->value < rhs : Op == 3 ? p->value <= rhs :
         Op == 4 ? p->value > rhs : p->value >= rhs;
}
template<unsigned Op, class T>
__attribute__((noinline)) int branch(const Cell<T>* p, T rhs)
{
  bool yes = Op == 0 ? p->value == rhs : Op == 1 ? p->value != rhs :
             Op == 2 ? p->value < rhs : Op == 3 ? p->value <= rhs :
             Op == 4 ? p->value > rhs : p->value >= rhs;
  if (yes) { ++hits; return 17; }
  misses += 3;
  return 29;
}
template<unsigned Op, class T>
__attribute__((noinline)) int literal_branch(const Cell<T>* p)
{
  if (Op == 0 ? p->value == T(7) : Op == 1 ? p->value != T(7) :
      Op == 2 ? p->value < T(7) : Op == 3 ? p->value <= T(7) :
      Op == 4 ? p->value > T(7) : p->value >= T(7)) {
    ++hits; return 31;
  }
  misses += 5;
  return 43;
}
template<unsigned Op, class T> void check(Cell<T>* p, T rhs)
{
  record(branch<Op>(p, rhs));
  record(predicate<Op>(p, rhs));
  record(literal_branch<Op>(p));
}
template<class T> void run()
{
  const U inputs[] = { 0, 1, 7, 127, 128, 255, 32767, 32768, 65535,
                       0x7fffffffULL, 0x80000000ULL, 0xffffffffffffffffULL,
                       0x7fffffffffffffffULL, 0x8000000000000000ULL };
  Cell<T> cell = {};
  for (unsigned i = 0; i < sizeof(inputs)/sizeof(inputs[0]); ++i) {
    cell.value = static_cast<T>(inputs[i]);
    for (unsigned j = 0; j < sizeof(inputs)/sizeof(inputs[0]); ++j) {
      T rhs = static_cast<T>(inputs[j]);
      check<0>(&cell, rhs); check<1>(&cell, rhs); check<2>(&cell, rhs);
      check<3>(&cell, rhs); check<4>(&cell, rhs); check<5>(&cell, rhs);
    }
  }
}
int main()
{
  run<signed char>(); run<unsigned char>();
  run<short>(); run<unsigned short>();
  run<int>(); run<unsigned>();
  run<long long>(); run<unsigned long long>();
  return digest == 4104275186648348043ULL && hits == 9408U && misses == 37632U ? 0 : 1;
}

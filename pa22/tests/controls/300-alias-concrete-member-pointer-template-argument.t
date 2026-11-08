// N3485 14.3.3p1/14.5.7/5.5: a real template argument forms the declared member pointer; expect exit 0
struct symbol_set {};
template<class...> struct items { int value; };
struct Term { int sum; void multiply(items<int>& value, const symbol_set&) { sum += value.value; } };
template<class T, template<class...> class TT, class... Args>
using bar = void (T::*)(TT<Args...>&, const symbol_set&);
int main() {
  bar<Term, items, int> operation = &Term::multiply;
  Term term = {3};
  items<int> value = {7};
  symbol_set symbols;
  (term.*operation)(value, symbols);
  return term.sum - 10;
}

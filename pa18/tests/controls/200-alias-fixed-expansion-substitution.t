// N3485 14.8.2/14.5.7; invalid concrete arity drops a candidate, valid nonempty arity participates; expect exit 0
template<class A, class B> struct Pair {};
template<class A, class B> using Both = Pair<A, B>;
template<class... T> int choose(Both<T..., int>*) { return 7; }
template<class... T> int choose(...) { return 11; }
int main() {
  Pair<char, int>* argument = 0;
  Pair<long, int>* incompatible = 0;
  return choose<>(0) + choose<char>(argument) + choose<char, long>(0) +
         choose<char>(incompatible) - 40;
}

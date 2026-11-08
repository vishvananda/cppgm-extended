struct Graph {
  template<int N> static bool check(int value) { return value == N; }
};
template<class Predicate> bool apply(Predicate predicate) { return predicate(7); }
struct Counter { int check; };
int main() {
  Graph graph;
  bool (*pointer)(int) = graph.check<7>;
  bool (*pointers[2])(int) = {graph.check<7>, graph.check<8>};
  Counter counter{3};
  if (!(counter.check < 4 && counter.check > 2)) return 1;
  return pointer(7) && pointers[0](7) && !pointers[1](7) &&
         apply(graph.check<7>) && apply((graph.check<7>)) ? 0 : 2;
}

// N3485 14.5.3p4-p5, 14.5.7: aliases retain valid nested expansions,
// function parameter packs, sizeof..., dependent expressions and qualifiers.
// expect exit 0
template<class... Types> struct List { static const int count = sizeof...(Types); };
template<class... Types> using Expanded = List<Types*...>;
template<class... Types> using Function = int (*)(Types...);
template<class... Types> using Count = int[sizeof...(Types) + 1];
template<int N> struct Number { static const int value = N; };
template<class... Types> using Sizes = List<Number<sizeof(Types)>...>;
template<int... Values> using Numbers = List<Number<Values>...>;
template<class... Types> using Nested = List<List<Types...>, int>;
template<class... Types> using Result = decltype(sizeof...(Types));
template<class> using Discard = int;
template<class... Types> using Discarded = List<Discard<Types>...>;
struct Member { using Type = int; };
template<class... Types> using Members = List<typename Types::Type...>;
int sum(int a, int b) { return a + b; }
int main() {
  Function<int, int> f = sum;
  Count<char, int> count = {};
  Result<int, char> size = 2;
  if (f(3, 4) != 7 || sizeof(count) != 3 * sizeof(int) || size != 2) return 1;
  if (Expanded<>::count != 0 || Expanded<char, int>::count != 2) return 2;
  if (Sizes<char, int>::count != 2 || Numbers<1, 2, 3>::count != 3) return 3;
  if (Nested<char, int>::count != 2 || Discarded<char, int>::count != 2) return 4;
  return Members<Member>::count == 1 ? 0 : 5;
}

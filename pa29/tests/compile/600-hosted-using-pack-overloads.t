template<class...> struct Types {};
template<int...> struct Indices {};
template<int N, class T> struct Candidate {
  static constexpr int choose(T) { return N; }
};

template<class, class> struct Overloads;
template<class... T, int... N>
struct Overloads<Types<T...>, Indices<N...>> : Candidate<N, T>... {
  using Candidate<N, T>::choose...;
};

using Set = Overloads<Types<int, double, char>, Indices<4, 7, 9>>;
static_assert(Set::choose(1) == 4, "first expanded overload");
static_assert(Set::choose(1.0) == 7, "middle expanded overload");
static_assert(Set::choose('a') == 9, "last expanded overload");

template<class... T> struct Import : T... { using T::choose...; };
static_assert(Import<Candidate<3, int>, Candidate<5, double>>::choose(1.0) == 5,
              "direct type pack");
static_assert(sizeof(Import<>) > 0, "empty using expansion");
static_assert(sizeof(Overloads<Types<>, Indices<>>) > 0, "empty zipped expansion");

struct DisabledA { void choose() = delete; };
struct DisabledB { void choose() = delete; };
static_assert(Import<DisabledA, DisabledB, Candidate<8, int>>::choose(4) == 8,
              "identical inherited signatures need no call");

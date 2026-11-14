// expect exit 0
// N3485 14.5.3, 14.8.2: a member template's default argument can expand
// its enclosing class's bound pack while still depending on its own parameter.
template<bool...> struct All;
template<> struct All<> { static const bool value = true; };
template<bool B, bool... Rest> struct All<B, Rest...> {
  static const bool value = B && All<Rest...>::value;
};
template<class T> struct Large { static const bool value = sizeof(T) > 1; };
template<class T, bool> struct Dependent : T {};
template<bool, class T = int> struct Enable {};
template<class T> struct Enable<true, T> { typedef T type; };

template<class... Types> struct Choice {
  template<bool Dummy = true,
           typename Enable<All<(Dependent<Large<Types>, Dummy>::value &&
                                 Dependent<Large<Types>, Dummy>::value)...>::value>::type = 0>
  int pick(int) { return sizeof...(Types); }
  int pick(...) { return -1; }
};

int main() {
  Choice<int, long> two;
  Choice<> empty;
  Choice<char> small;
  return two.pick(0) == 2 && empty.pick(0) == 0 && small.pick(0) == -1 ? 0 : 1;
}

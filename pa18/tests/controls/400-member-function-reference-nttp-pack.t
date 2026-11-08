// expect exit 0
// N3485 14.1p8, 14.3.2p5, 14.3.3p3: typed template parameter normalization.
int target(int value) { return value + 1; }
template<class F> struct Outer;
template<class R, class... Args> struct Outer<R(Args...)> {
  template<R (&function)(Args...)> struct Call { static int run(int value) { return function(value); } };
};
int main() { return Outer<int(int)>::Call<target>::run(3) == 4 ? 0 : 1; }

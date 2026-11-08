// expect reject
// N3485 14.1p8, 14.3.2p5, 14.3.3p3: typed template parameter normalization.
int target(float value) { return 0; }
template<class F> struct Outer;
template<class R, class... Args> struct Outer<R(Args...)> { template<R (&function)(Args...)> struct Call {}; };
Outer<int(int)>::Call<target> value;

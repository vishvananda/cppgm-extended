template<int N> struct Value { static __constinit int number; };
template<int N> int Value<N>::number = N;
template<int N> __constinit int variable = N;
template<class T> __constinit T zero;
int use() { return ++Value<3>::number + variable<2> + zero<int>; }

struct Literal { int number; constexpr Literal() : number(5) {} };
template<class T> int function_value() { static __constinit T value; return value.number; }
int use_function() { return function_value<Literal>(); }

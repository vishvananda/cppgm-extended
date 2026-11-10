template<auto N> struct Constant { static constexpr decltype(N) value=N; };
int main() { return sizeof(Constant<3L>::value)==sizeof(long) && Constant<3>::value==3?0:1; }

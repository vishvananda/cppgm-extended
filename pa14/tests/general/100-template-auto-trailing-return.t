// HHC-090
template<class T1, class T2>
inline auto diff(const T1& x, const T2& y) -> decltype(y - x) {
  return y - x;
}

int main() { return 0; }

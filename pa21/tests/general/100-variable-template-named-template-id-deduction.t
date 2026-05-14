template<long N, long D>
struct ratio {};

template<class T>
inline const bool is_ratio = false;

template<long N, long D>
inline const bool is_ratio<ratio<N, D>> = true;

using nano = ratio<1, 1000>;

static_assert(is_ratio<nano>, "");

int main() {
  return 0;
}

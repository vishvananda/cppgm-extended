template <class A, class B = A>
struct Box {};

template <class T>
struct Box<T, T> {};

template <class T, class U = T>
using Alias = Box<T, U>;

template <class A, class B = A>
inline constexpr int score = 1;

template <class T>
inline constexpr int score<T, T> = 2;

template <class T, int Gate = 0>
int choose(T) {
  return Gate;
}

template <class T>
long choose(T *) {
  return 7;
}

int main() {
  Box<char> same_box;
  Alias<long> alias_box = {};
  static_assert(score<int> == 2, "partial variable specialization should win");
  int primary = choose(3);
  long pointer = choose((int *)0);
  int explicit_call = choose<long, 1>(4L);
  (void)same_box;
  (void)alias_box;
  return primary + static_cast<int>(pointer) + explicit_call + score<int>;
}

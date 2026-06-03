template<class T>
using decay_t = __decay(T);

template<class T>
decay_t<T> decay_copy(T&&) {
  return decay_t<T>();
}

struct S {
  void g();
};

void S::g() {
  struct G {};
  decay_copy(G{});
}

int main() {
  S s;
  s.g();
  return 0;
}

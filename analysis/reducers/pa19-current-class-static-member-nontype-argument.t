template<unsigned long long A,
         unsigned long long B,
         unsigned long long C,
         unsigned long long D>
struct helper {
  typedef unsigned long long result_type;
  static result_type next(result_type x) { return x + D; }
};

template<class T, T A, T B, T C>
struct engine {
  typedef T result_type;
  static const result_type M = result_type(-1);
  typedef helper<A, B, C, M> helper_type;
};

int accept(helper<1, 0, 2, engine<unsigned, 1, 0, 2>::M> *);

int main() {
  engine<unsigned, 1, 0, 2>::helper_type *p = 0;
  return accept(p);
}

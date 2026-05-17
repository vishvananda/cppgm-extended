struct sink {
  int value;
  void accept(int const & x) { value = x; }
};

struct matcher {
  int x;
  matcher(int v = 0) : x(v) {}
};

struct iface {
  virtual ~iface() {}
  virtual void peek(sink &) const = 0;
};

template<typename M>
struct dynamic : M, iface {
  int tail;
  dynamic(M const & m) : M(m), tail(9) {}
  void peek(sink & out) const { out.accept(static_cast<M const *>(this)->x); }
};

int main()
{
  dynamic<matcher> d(matcher(42));
  iface const * p = &d;
  sink out = {0};
  p->peek(out);
  return out.value == 42 ? 0 : 1;
}

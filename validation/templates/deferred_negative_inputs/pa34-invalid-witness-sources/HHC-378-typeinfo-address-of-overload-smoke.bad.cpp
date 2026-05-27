struct X {
  bool same(const X& other) const {
    return this == &other;
  }
};

int operator&(int, int);

int main() {
  X x;
  return x.same(x) ? 0 : 1;
}

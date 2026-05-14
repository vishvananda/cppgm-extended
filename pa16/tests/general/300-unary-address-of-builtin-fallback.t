struct X {
  bool same(const X& other) const {
    return this == &other;
  }
};

struct Y {};
int operator&(Y, Y);

int main() {
  X x;
  return x.same(x) ? 0 : 1;
}

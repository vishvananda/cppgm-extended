struct S {
  S();
  S(const S&);
  ~S();
};

S::S() {}
S::S(const S&) {}
S::~S() {}

S f() { return S(); }
S g() { return S(); }

int main() {
  bool c = true;
  S s = c ? f() : g();
  return 0;
}

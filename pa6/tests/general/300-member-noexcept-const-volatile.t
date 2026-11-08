// CXX11-128: implicit this in an exception specification has the function's cv.
// N3485 5.1.1p3,5.2.5,5.3.7,15.4; GCC PR c++/92907 neighbor.
void probe(const volatile int&) noexcept;
void probe(int&) = delete;
struct Holder {
  int i;
  void inspect() const volatile noexcept(noexcept(probe(i)));
};
int main() { return 0; }
